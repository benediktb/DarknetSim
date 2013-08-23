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
#include <IPAddressResolver.h>
#include "DarknetBaseNode.h"
#include <UDPPacket.h>
#include <cstringtokenizer.h>
#include <algorithm>


void DarknetBaseNode::addPeer(std::string nodeID, IPvXAddress& destAddr, int destPort) {
    DarknetPeer* peer = new DarknetPeer;
    peer->nodeID = nodeID;
    peer->address = destAddr;
    peer->port = destPort;
    peers.insert(std::pair<std::string, DarknetPeer*>(nodeID,peer));
}

void DarknetBaseNode::initialize(int stage) {
    switch (stage) {
    case 0:
        nodeID = par("nodeID").stdstringValue();
        localPort = par("localPort");
        bindToPort(localPort);
        break;
    case 3: {
        std::vector<std::string> v = cStringTokenizer(par("destinations")).asVector();
        for(std::vector<std::string>::iterator iter = v.begin(); iter != v.end(); iter++) {
            std::vector<std::string> peer_tuple = cStringTokenizer((*iter).c_str(),":").asVector(); //split <destID>:<destPort>
            if(peer_tuple.size() == 2) {
                std::string nodeID = peer_tuple[0];
                std::istringstream convert(peer_tuple[1]);
                int port;
                port = convert >> port ? port : 0;  //convert string to int (user 0 on error)
                IPvXAddress ip = IPAddressResolver().resolve(nodeID.c_str());
                addPeer(nodeID, ip, port);
            }else {
                EV << "Error on parsing peer list; this peer seems malformed: " << (*iter);
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

void DarknetBaseNode::sendPacket(DarknetMessage* dmsg, IPvXAddress& destAddr, int destPort) {
    sendToUDP(dmsg, localPort, destAddr, destPort);
}

/*
 * sends a DarknetMessage directly to its destination.
 * if destination is not in peers list, just drop it
 */
bool DarknetBaseNode::sendDirectMessage(DarknetMessage* msg) {
    if(peers.find(msg->getDestNodeID()) != peers.end()) {
        DarknetPeer *peer = peers[msg->getDestNodeID()];
        msg->setSrcNodeID(nodeID.c_str());
        sendPacket(msg,peer->address,peer->port);
        return true;
    }else { //destination node not in peers list -> not possible to send direct message
        EV << "destination node(" << msg->getDestNodeID() << " not in peers list -> not possible to send direct message: " << msg;
        return false;
    }
}

bool DarknetBaseNode::sendMessage(DarknetMessage* msg) {
    std::vector<DarknetPeer*> destPeers = findNextHop(msg);
    if(/*destPeers != NULL and*/ destPeers.size() > 0) {
        msg->setSrcNodeID(nodeID.c_str());
        for(std::vector<DarknetPeer*>::iterator iter = destPeers.begin(); iter != destPeers.end(); iter++) {
            sendPacket(msg->dup(),(*iter)->address,(*iter)->port);
        }
        delete msg;
        return true;
    } else {
        EV << "No next hop found for message: " << msg << endl;
     //   if(destPeers != NULL)
            EV << "size of destPeers: " << destPeers.size() << endl;
        //TODO: implement proper default error handling here
        delete msg;
        return false;
    }
}


void DarknetBaseNode::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    } else {
        DarknetMessage* dm = dynamic_cast<DarknetMessage*>(msg);
        if( dm != NULL) {
            handleDarknetMessage(dm);
        }
        else {
            EV << "received an unknown cMessage: " << msg;
            delete msg;
        }
    }
}

void DarknetBaseNode::handleDarknetMessage(DarknetMessage *msg) {
    if(msg->getDestNodeID() != nodeID) { // message for another node -> forward it
        forwardMessage(msg);
    }else if(forwardedIdTable.find(msg->getRequestMessageID()) != forwardedIdTable.end() and std::find(outstandingResponses.begin(),outstandingResponses.end(),msg->getRequestMessageID()) == outstandingResponses.end()) {
        // response for an forwarded Message
        forwardResponse(msg);
    }else {     // message for this Node
        handleIncomingMessage(msg);
    }
}

void DarknetBaseNode::handleIncomingMessage(DarknetMessage *msg) {
    switch(msg->getType()) {
    case DM_REQUEST:
        handleRequest(msg);
        break;
    case DM_RESPONSE:
        EV << "host: " << nodeID <<" > recieved PONG from: " << msg->getSrcNodeID() << endl;
        delete msg;
        break;
     default:
       EV << "received unknown DarknetMessage for this node: " << msg << ": " << msg->getType() << endl;
       delete msg;
       break;
    }
}

/*
 * if TTL is > 0; insert the messages treeID in forwardedIdTable and forward it (after decrement TTL)
 * drop it if TTL is = 0
 */
void DarknetBaseNode::forwardMessage(DarknetMessage* msg) {
    int ttl = msg->getTTL();
    if(ttl > 0) {
        msg->setTTL(ttl-1); //TODO: add probability of TTL reduction
        forwardedIdTable.insert(std::pair<long, std::string>(msg->getTreeId(),msg->getSrcNodeID()));
        sendMessage(msg);
    }else {
        // TODO: inform simulator/user of droped message (or at least count it)
        EV << "dropped message";
        delete msg;
    }
}

/*
 * forward a Response to a previously forwarded Message back its path "up"
 */
void DarknetBaseNode::forwardResponse(DarknetMessage* msg) {
    //if(midTab.find(msg->getRequestMessageID()) != midTab.end()) {
        msg->setDestNodeID(forwardedIdTable[msg->getRequestMessageID()].c_str());
        forwardedIdTable.erase(msg->getRequestMessageID());
        sendMessage(msg);
/*    } else { // this should not happen, probably a bug
        EV << "can not forward message - no entry in midTab found!";
        delete msg;
    } */
}

void DarknetBaseNode::handleRequest(DarknetMessage* request) {
    DarknetMessage *msg = new DarknetMessage();
    msg->setDestNodeID(request->getSrcNodeID());
    msg->setType(DM_RESPONSE);
    msg->setRequestMessageID(request->getTreeId());
    delete request;
    sendMessage(msg);
}


DarknetMessage* DarknetBaseNode::makeRequest(std::string nodeID) {
    DarknetMessage *msg = new DarknetMessage();
    msg->setDestNodeID(nodeID.c_str());
    msg->setType(DM_REQUEST);
    outstandingResponses.push_back(msg->getTreeId());
    return msg;
}
