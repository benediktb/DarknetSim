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

#include "DarknetOfflineDetectionNode.h"
#include <cstdio>

void DarknetOfflineDetectionNode::initialize(int stage) {
    DarknetSimpleNode::initialize(stage);
    switch(stage) {
    case 0:
        sigDropResendExeeded = registerSignal("sigDropResendExeeded");
        break;
    case 4:
        resendCounter = par("resendCounter");
        resendTimerMean = par("resendTimerMean");
        resendTimerVariance = par("resendTimerVariance");
        break;
    }
}

/*
 * initialize connection establishment by sending a DM_CON_SYN
 */
void DarknetOfflineDetectionNode::connectPeer(std::string nodeID) {
    DarknetMessage *dm = new DarknetMessage();
    dm->setType(DM_CON_SYN);
    dm->setTTL(defaultTTL);
    dm->setDestNodeID(nodeID.c_str());
    sendDirectMessage(dm);
}

void DarknetOfflineDetectionNode::handleIncomingMessage(DarknetMessage *msg, DarknetPeer *sender) {
    switch(msg->getType()) {
    case DM_CON_SYN: {
        if(friendsByID.find(msg->getSrcNodeID()) != friendsByID.end()) {
            EV << "Received CON_SYN from: " << msg->getSrcNodeID() << endl;
            DarknetMessage *ack = new DarknetMessage();
            ack->setType(DM_CON_ACK);
            ack->setTTL(defaultTTL);
            ack->setDestNodeID(msg->getSrcNodeID());
            sendDirectMessage(ack);
        }
        delete msg;
        break;
    }
    case DM_CON_ACK: {
        addActivePeer(msg->getSrcNodeID());
        delete msg;
        break;
    }
    default:
        DarknetBaseNode::handleIncomingMessage(msg, sender);
       break;
    }
}

void DarknetOfflineDetectionNode::addActivePeer(std::string nodeId) {
    if(connected.count(nodeID) == 0) {
        connected.insert(nodeID);
        EV << "connection to " << nodeID << "established" << endl;
    }
}


/*
 * check if message is of type DM_RCVACK, if so stop resendTimer for according message.
 * if not, send DM_RCVACK to sender and pass it to handleIncomingMessage
 */
void DarknetOfflineDetectionNode::handleDarknetMessage(DarknetMessage* msg, DarknetPeer *sender) {
    if (msg->getType() == DM_RCVACK) {
        handleRcvAck(msg);
    } else {
        sendRcvAck(msg);
        DarknetSimpleNode::handleDarknetMessage(msg, sender);
    }
}

void DarknetOfflineDetectionNode::handleRcvAck(DarknetMessage* msg) {
    long orig_mID = msg->getRequestMessageID();
    if(rcvack_waiting.count(orig_mID) == 1) {
        EV << "received RCVACK for message: " << rcvack_waiting[orig_mID].first->getType() << " (id:" <<rcvack_waiting[orig_mID].first->getId() << ")"  << endl;
        cancelAndDelete(rcvack_waiting[orig_mID].first);
        rcvack_waiting.erase(orig_mID);
    }
    delete msg;
}

void DarknetOfflineDetectionNode::sendRcvAck(DarknetMessage* msg) {
    EV << "send RCVACK for message: " << msg->getType() << " (ID:" << msg->getId() << "/treeID: " << msg->getTreeId() << ")"  << endl;
    DarknetMessage* ack = new DarknetMessage();
    ack->setDestNodeID(msg->getSrcNodeID());
    ack->setType(DM_RCVACK);
    ack->setTTL(defaultTTL);
    ack->setRequestMessageID(msg->getId());
    sendDirectMessage(ack);
}

void DarknetOfflineDetectionNode::removeInactivePeer(std::string peerId) {
    connected.erase(peerId);
}

/*
 * check if msg is a DarknetMessage in the list of sent messages waiting for an RCVACK.
 * if so, check if resendCounter is reached and otherwise resend (and reschedule it)
 */
void DarknetOfflineDetectionNode::handleSelfMessage(cMessage* msg) {
    DarknetMessage* dm = dynamic_cast<DarknetMessage*>(msg);
    if(dm != NULL and dm->hasPar("origMsgId")  and rcvack_waiting.count(dm->par("origMsgId").longValue()) == 1) {
        long msgID = dm->par("origMsgId").longValue();

        if(rcvack_waiting[msgID].second < resendCounter) {
            int destPort = (int) dm->par("destPort").longValue();
            IPvXAddress* destAddr = (IPvXAddress*) (dm->par("destAddr").pointerValue());
            DarknetMessage* dup = dm->dup();
            DarknetSimpleNode::sendPacket(dup, *destAddr, destPort);
            rcvack_waiting[msgID].second++;
            dm->par("origMsgID").setLongValue(dup->getId());
            rcvack_waiting.insert(std::pair<long, std::pair<DarknetMessage*,int> >(dup->getId(), rcvack_waiting[msgID]));
            rcvack_waiting.erase(msgID);
            scheduleAt(simTime() + normal(resendTimerMean,resendTimerVariance), dm);
        } else { /* too many resends; delete resendTimer and remove peer from the connected list */
            EV << "stop resendTimer for message: " << msg << " and remove the peer" << endl;
            emit(sigDropResendExeeded,rcvack_waiting[msgID].first->getTTL());
            removeInactivePeer(dm->getDestNodeID());
            rcvack_waiting.erase(msgID);

            delete dm;
        }
    } else DarknetSimpleNode::handleSelfMessage(msg);
}

void DarknetOfflineDetectionNode::sendPacket(DarknetMessage* pkg, IPvXAddress& destAddr, int destPort) {

    DarknetMessage* dup = pkg->dup();
    dup->addPar("origMsgID");
    dup->par("origMsgID").setLongValue(pkg->getId());
    dup->addPar("destAddr");
    dup->addPar("destPort");
    dup->par("destAddr").setPointerValue(&destAddr);
    dup->par("destPort").setLongValue(destPort);
    EV << "start resend timer for message: (id:" << pkg->getId() << ", DestID: "<<pkg->getDestNodeID() << ")"  << endl;
    rcvack_waiting.insert(std::pair<long, std::pair<DarknetMessage*,int> >(pkg->getId(), std::pair<DarknetMessage*,int>(dup,0)));
    scheduleAt(simTime() + normal(resendTimerMean, resendTimerVariance), dup);

    DarknetSimpleNode::sendPacket(pkg, destAddr, destPort);
}
