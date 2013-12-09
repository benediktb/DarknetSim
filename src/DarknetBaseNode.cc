//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <omnetpp.h>
#include <IPvXAddressResolver.h>
#include "DarknetBaseNode.h"
#include <UDPPacket.h>
#include <UDPControlInfo.h>
#include <cstringtokenizer.h>
#include <algorithm>

void DarknetBaseNode::addPeer(std::string nodeID, IPvXAddress& destAddr,
        int destPort) {
    DarknetPeer* peer = new DarknetPeer;
    UDPAddress* address = new UDPAddress(destAddr, destPort);
    peer->nodeID = nodeID;
    peer->address = *address;
    peers.insert(std::pair<std::string, DarknetPeer*>(nodeID, peer));
    peersByAddress.insert(std::pair<UDPAddress, DarknetPeer*>(*address, peer));
}

void DarknetBaseNode::initialize(int stage) {
    AppBase::initialize(stage);

    switch (stage) {
    case 0:
        localPort = par("localPort");
        socket.setOutputGate(gate("udpOut"));
        socket.bind(localPort);

        nodeID = par("nodeID").stdstringValue();
        defaultTTL = par("defaultTTL");
        sigSendDM = registerSignal("sigSendDM");
        sigUnhandledMSG = registerSignal("sigUnhandledMSG");
        sigDropTtlExeeded = registerSignal("sigDropTtlExeeded");
        sigRequestRemainingTTL = registerSignal("sigRequestRemainingTTL");
        sigResponseRemainingTTL = registerSignal("sigResponseRemainingTTL");
        break;
    case 3: {
        std::vector<std::string> v =
                cStringTokenizer(par("friends")).asVector();
        for (std::vector<std::string>::iterator iter = v.begin();
                iter != v.end(); iter++) {
            std::vector<std::string> peer_tuple = cStringTokenizer(
                    (*iter).c_str(), ":").asVector(); //split <destID>:<destPort>
            if (peer_tuple.size() == 3) {
                std::string moduleName = peer_tuple[0];
                std::string nodeID = peer_tuple[1];
                std::istringstream convert(peer_tuple[2]);
                int port;
                port = convert >> port ? port : 0; //convert string to int (user 0 on error)
                IPvXAddress ip = IPvXAddressResolver().resolve(moduleName.c_str());
                addPeer(nodeID, ip, port);
            } else {
                EV<< "Error on parsing peer list; this peer seems malformed: " << (*iter) << endl;
            }
        }}
    break;
    case 4:
    for(std::map<std::string, DarknetPeer*>::iterator iter = peers.begin(); iter != peers.end(); iter++) {
        connectPeer(iter->second->nodeID);
    }
    break;
}
}

void DarknetBaseNode::sendToUDP(DarknetMessage *msg, int srcPort,
        const IPvXAddress& destAddr, int destPort) {
    EV << "Sending UDP packet: " << getParentModule()->getFullName() << " --> ";
    EV << msg->toString() << "  (" << msg->getByteLength() << " bytes)";
    EV << " --> " << IPvXAddressResolver().findHostWithAddress(destAddr)->getFullName();
    EV << " (" << destAddr << ":" << destPort << ")" << endl;

    socket.sendTo(msg, destAddr, destPort);
}

void DarknetBaseNode::sendPacket(DarknetMessage* dmsg, IPvXAddress& destAddr,
        int destPort) {
    emit(sigSendDM, dmsg->getTreeId());
    sendToUDP(dmsg, localPort, destAddr, destPort);
}

/*
 * sends a DarknetMessage directly to its destination.
 * if destination is not in peers list, just drop it
 */
bool DarknetBaseNode::sendDirectMessage(DarknetMessage* msg) {
    if (peers.find(msg->getDestNodeID()) != peers.end()) {
        DarknetPeer *peer = peers[msg->getDestNodeID()];
        msg->setSrcNodeID(nodeID.c_str());
        sendPacket(msg, peer->address.first, peer->address.second);
        return true;
    } else { //destination node not in peers list -> not possible to send direct message
        EV<< "destination node(" << msg->getDestNodeID() << " not in peers list -> not possible to send direct message: " << msg;
        return false;
    }
}

bool DarknetBaseNode::sendMessage(DarknetMessage* msg) {
    std::vector<DarknetPeer*> destPeers = findNextHop(msg);
    if (/*destPeers != NULL and*/destPeers.size() > 0) {
        msg->setSrcNodeID(nodeID.c_str());
        for (std::vector<DarknetPeer*>::iterator iter = destPeers.begin();
                iter != destPeers.end(); iter++) {
            sendPacket(msg->dup(), (*iter)->address.first, (*iter)->address.second);
        }
        delete msg;
        return true;
    } else {
        EV<< "No next hop found for message: " << msg << endl;
        //TODO: implement proper default error handling here
        delete msg;
        return false;
    }
}

void DarknetBaseNode::handleUDPMessage(cMessage *msg) {
    DarknetMessage* dm = dynamic_cast<DarknetMessage*>(msg);
    if (dm != NULL) {
        UDPDataIndication* udi = (UDPDataIndication*) dm->getControlInfo();
        UDPAddress* srcAddr = new UDPAddress(udi->getSrcAddr(), udi->getSrcPort());
        std::map<UDPAddress, DarknetPeer*>::iterator it;
        it = peersByAddress.find(*srcAddr);
        if (peersByAddress.end() == it) {
            EV << "Could not get sending peer for message: " << msg << endl;

            EV << "  + Source address: " << srcAddr->first.str() << ":" << srcAddr->second << endl;

            EV << "  + Content of address -> peer map: " << endl;
            typedef std::map<UDPAddress, DarknetPeer*>::iterator it_type;
            for(it_type iterator = peersByAddress.begin(); iterator != peersByAddress.end(); iterator++) {
                EV << "    " << iterator->first.first.str() << ":" << iterator->first.second << " -> " << iterator->second->nodeID << endl;
            }

            delete msg;
            return;
        }
        DarknetPeer* sender = it->second;
        EV << "Received DarknetMessage from peer " << sender->nodeID << endl;
        handleDarknetMessage(dm, sender);
    } else {
        EV<< "received an unknown cMessage: " << msg << endl;
        delete msg;
    }
}

void DarknetBaseNode::handleDarknetMessage(DarknetMessage *msg, DarknetPeer *sender) {
    if (msg->getDestNodeID() != nodeID) { // message for another node -> forward it
        forwardMessage(msg, sender);
    } else if (forwardedIdTable.find(msg->getRequestMessageID())
            != forwardedIdTable.end()
            and std::find(outstandingResponses.begin(),
                    outstandingResponses.end(), msg->getRequestMessageID())
                    == outstandingResponses.end()) {
        // response for an forwarded Message
        forwardResponse(msg);
    } else {     // message for this Node
        handleIncomingMessage(msg, sender);
    }
}

void DarknetBaseNode::handleIncomingMessage(DarknetMessage *msg, DarknetPeer *sender) {
    switch (msg->getType()) {
    case DM_REQUEST:
        emit(sigRequestRemainingTTL, msg->getTTL());
        handleRequest(msg, sender);
        break;
    case DM_RESPONSE:
        emit(sigResponseRemainingTTL, msg->getTTL());
        outstandingResponses.erase(msg->getRequestMessageID());
        delete msg;
        break;
    default:
        emit(sigUnhandledMSG, msg->getId());
        delete msg;
        break;
    }
}

bool DarknetBaseNode::canMessageBeForwarded(DarknetMessage* msg) {
    return msg->getTTL() > 0;
}

void DarknetBaseNode::doForwardingChangesOnMessage(DarknetMessage* msg) {
    msg->setTTL(msg->getTTL() - 1); //TODO: add probability of TTL reduction
}

/*
 * if TTL is > 0; insert the messages treeID in forwardedIdTable and forward it (after decrement TTL)
 * drop it if TTL is = 0
 */
void DarknetBaseNode::forwardMessage(DarknetMessage* msg, DarknetPeer *sender) {
    if (canMessageBeForwarded(msg)) {
        doForwardingChangesOnMessage(msg);
        forwardedIdTable.insert(
                std::pair<long, std::string>(msg->getTreeId(),
                        msg->getSrcNodeID()));
        sendMessage(msg);
    } else {
        // TODO: inform simulator/user of droped message (or at least count it)
        EV<< "dropped message";
        emit(sigDropTtlExeeded,msg->getTreeId());
        delete msg;
    }
}

        /*
         * forward a Response to a previously forwarded Message back its path "up"
         */
void DarknetBaseNode::forwardResponse(DarknetMessage* msg) {
    msg->setDestNodeID(forwardedIdTable[msg->getRequestMessageID()].c_str());
    forwardedIdTable.erase(msg->getRequestMessageID());
    sendMessage(msg);
}

void DarknetBaseNode::handleRequest(DarknetMessage* request, DarknetPeer *sender) {
    DarknetMessage *msg = new DarknetMessage();
    msg->setDestNodeID(request->getSrcNodeID());
    msg->setType(DM_RESPONSE);
    msg->setTTL(defaultTTL);
    msg->setRequestMessageID(request->getTreeId());
    delete request;
    sendMessage(msg);
}

DarknetMessage* DarknetBaseNode::makeRequest(DarknetMessage *msg, std::string nodeID) {
    msg->setDestNodeID(nodeID.c_str());
    msg->setType(DM_REQUEST);
    msg->setTTL(defaultTTL);
    outstandingResponses.insert(msg->getTreeId());
    return msg;
}

DarknetMessage* DarknetBaseNode::makeRequest(std::string nodeID) {
    DarknetMessage *msg = new DarknetMessage();
    return makeRequest(msg, nodeID);
}

void DarknetBaseNode::handleMessageWhenUp(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    } else if (msg->getKind() == UDP_I_DATA) {
        // process incoming packet
        handleUDPMessage(msg);
    } else if (msg->getKind() == UDP_I_ERROR) {
        EV<< "Ignoring UDP error report\n";
        delete msg;
    }
    else {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

}

