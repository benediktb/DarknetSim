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
    DarknetBaseNode::initialize(stage);
    switch (stage) {
    case 0:
        rcvack_waiting.clear();
        sigDropResendExeeded = registerSignal("sigDropResendExeeded");
        sigRetransmissionAfterTimeout = registerSignal(
                "sigRetransmissionAfterTimeout");
        resendCounter = par("resendCounter");
        resendTimerMean = par("resendTimerMean");
        resendTimerVariance = par("resendTimerVariance");
        break;
    case 4:
        break;
    }
}

bool DarknetOfflineDetectionNode::startApp(IDoneCallback *doneCallback) {
    return DarknetBaseNode::startApp(doneCallback);
}

bool DarknetOfflineDetectionNode::stopApp(IDoneCallback *doneCallback) {
    return crashApp(doneCallback);
}

void DarknetOfflineDetectionNode::cancelAllRetransmissions() {
    // Cancel all pending timeouts
    std::map<long, std::pair<DarknetMessage*, int> >::iterator it;
    for (it = rcvack_waiting.begin(); it != rcvack_waiting.end(); it++) {
        cancelAndDelete(it->second.first);
    }
    rcvack_waiting.clear();
}

bool DarknetOfflineDetectionNode::crashApp(IDoneCallback *doneCallback) {
    cancelAllRetransmissions();
    return DarknetBaseNode::crashApp(doneCallback);
}

/*
 * initialize connection establishment by sending a DM_CON_SYN
 */
void DarknetOfflineDetectionNode::connectPeer(std::string nodeID) {
    DarknetMessage *dm = new DarknetMessage("CON_SYN");
    dm->setType(DM_CON_SYN);
    dm->setTTL(defaultTTL);
    dm->setDestNodeID(nodeID.c_str());
    sendDirectMessage(dm);
}

void DarknetOfflineDetectionNode::handleIncomingMessage(DarknetMessage *msg,
        DarknetPeer *sender) {
    switch (msg->getType()) {
    case DM_CON_SYN: {
        std::map<std::string, DarknetPeer*>::iterator frIt = friendsByID.find(msg->getSrcNodeID());
        if (frIt != friendsByID.end()) {
            DEBUG("Received CON_SYN from: " << msg->getSrcNodeID() << endl);
            DarknetMessage *ack = new DarknetMessage("CON_SYNACK");
            ack->setType(DM_CON_SYNACK);
            ack->setTTL(defaultTTL);
            ack->setDestNodeID(msg->getSrcNodeID());
            sendDirectMessage(ack);
        }
        delete msg;
        msg = NULL;
        break;
    }
    case DM_CON_SYNACK: {
        std::map<std::string, DarknetPeer*>::iterator frIt = friendsByID.find(msg->getSrcNodeID());
        if (frIt != friendsByID.end()) {
            DEBUG("Received CON_SYNACK from: " << msg->getSrcNodeID() << endl);
            DarknetMessage *ack = new DarknetMessage("CON_ACK");
            ack->setType(DM_CON_ACK);
            ack->setTTL(defaultTTL);
            ack->setDestNodeID(msg->getSrcNodeID());
            sendDirectMessage(ack);
            if (not frIt->second->connected) {
                addActivePeer(msg->getSrcNodeID());
            }
        }
        delete msg;
        msg = NULL;
        break;
    }
    case DM_CON_ACK: {
        std::map<std::string, DarknetPeer*>::iterator frIt = friendsByID.find(msg->getSrcNodeID());
        if (not frIt->second->connected) {
            addActivePeer(msg->getSrcNodeID());
        }
        delete msg;
        msg = NULL;
        break;
    }
    default:
        DarknetBaseNode::handleIncomingMessage(msg, sender);
        break;
    }
}

void DarknetOfflineDetectionNode::addActivePeer(std::string nodeId) {
    DarknetPeer* peer = friendsByID.at(peerId);
    if (not peer->connected) {
        peer->connected = true;
        numConnected++;
        DEBUG("Connection to " << nodeId << " established" << endl);
    }
}

/*
 * check if message is of type DM_RCVACK, if so stop resendTimer for according message.
 * if not, send DM_RCVACK to sender and pass it to handleIncomingMessage
 */
void DarknetOfflineDetectionNode::handleDarknetMessage(DarknetMessage* msg,
        DarknetPeer *sender) {
    if (msg->getType() == DM_RCVACK) {
        handleRcvAck(msg);
    } else {
        sendRcvAck(msg);
        DarknetBaseNode::handleDarknetMessage(msg, sender);
    }
}

void DarknetOfflineDetectionNode::handleRcvAck(DarknetMessage* msg) {
    long orig_mID = msg->getRequestMessageID();
    std::map<long, std::pair<DarknetMessage*, int> >::iterator ackIt =
            rcvack_waiting.find(orig_mID);
    if (ackIt != rcvack_waiting.end()) {
        DarknetMessage* msgPendingAck = ackIt->second.first;

#ifdef USE_DEBUG
        DarknetMessageType type = msgPendingAck->getType();
        long msgId = msgPendingAck->getId();
        DEBUG(
                "Received RCVACK for message: " << type << " (id:" << msgId << ")" << endl);
#endif

        cancelAndDelete(msgPendingAck);
        rcvack_waiting.erase(orig_mID);
    }
    delete msg;
    msg = NULL;
}

void DarknetOfflineDetectionNode::sendRcvAck(DarknetMessage* msg) {
    DEBUG(
            "Send RCVACK for message: " << msg->getType() << " (ID:" << msg->getId() << "/treeID: " << msg->getTreeId() << ")" << endl);
    DarknetMessage* ack = new DarknetMessage("RCVACK");
    ack->setDestNodeID(msg->getSrcNodeID());
    ack->setType(DM_RCVACK);
    ack->setTTL(defaultTTL);
    ack->setRequestMessageID(msg->getId());
    sendDirectMessage(ack);
}

void DarknetOfflineDetectionNode::removeInactivePeer(std::string peerId) {
    DarknetPeer* peer = friendsByID.at(peerId);
    if (peer->connected) {
        peer->connected = false;
        numConnected--;
    }
}

/*
 * check if msg is a DarknetMessage in the list of sent messages waiting for an RCVACK.
 * if so, check if resendCounter is reached and otherwise resend (and reschedule it)
 */
void DarknetOfflineDetectionNode::handleSelfMessage(cMessage* msg) {
DarknetMessage* dm = dynamic_cast<DarknetMessage*>(msg);
if (dm != NULL and dm->hasPar("origMsgID")
        and rcvack_waiting.count(dm->par("origMsgID").longValue()) == 1) {
    long msgID = dm->par("origMsgID").longValue();
    std::pair<DarknetMessage*, int>* waiting = &rcvack_waiting[msgID];

    if (waiting->second < resendCounter) {
        int destPort = (int) dm->par("destPort").longValue();
        IPvXAddress* destAddr =
                (IPvXAddress*) (dm->par("destAddr").pointerValue());
        DarknetMessage* dup = dm->dup();
        DarknetBaseNode::sendPacket(dup, *destAddr, destPort);
        emit(sigRetransmissionAfterTimeout, dm->par("origMsgID").longValue());

        waiting->second++;
        waiting->first->par("origMsgID").setLongValue(dup->getId());
        rcvack_waiting.insert(std::make_pair(dup->getId(), *waiting));
        rcvack_waiting.erase(msgID);
        scheduleAt(simTime() + normal(resendTimerMean, resendTimerVariance),
                dm);
    } else {
        /* too many resends; delete resendTimer and remove peer from the connected list */
        DEBUG(
                "Stop resendTimer for message: " << msg << " and remove the peer" << endl);
        emit(sigDropResendExeeded, waiting->first->getTTL());
        removeInactivePeer(dm->getDestNodeID());
        rcvack_waiting.erase(msgID);

        delete dm;
        dm = NULL;
    }
}
}

void DarknetOfflineDetectionNode::sendPacket(DarknetMessage* pkg,
    IPvXAddress& destAddr, int destPort) {
// No ACKs for ACKs ... therefore no retransmissions. Also no ACKs for
//   connection establishment
if ((pkg->getType() != DM_RCVACK) and (pkg->getType() != DM_CON_SYN)
        and (pkg->getType() != DM_CON_SYNACK)
        and (pkg->getType() != DM_CON_ACK)) {
    DarknetMessage* dup = pkg->dup();

    dup->setName(CS(
            "Retransmission timeout of "
            << DarknetMessage::typeToString(pkg->getType())
            << " #" << pkg->getId()));

    dup->addPar("origMsgID");
    dup->par("origMsgID").setLongValue(pkg->getId());
    dup->addPar("destAddr");
    dup->addPar("destPort");
    dup->par("destAddr").setPointerValue(&destAddr);
    dup->par("destPort").setLongValue(destPort);

    DEBUG(
            "Start retransmission timer for message: (id:" << pkg->getId() << ", DestID: " << pkg->getDestNodeID() << ")" << endl);
    rcvack_waiting.insert(
            std::make_pair(pkg->getId(), std::make_pair(dup, (int) 0)));

    // Minimum reschedule time 10ms
    double rescheduleTime = std::max(
            normal(resendTimerMean, resendTimerVariance), 0.01);

    scheduleAt(simTime() + rescheduleTime, dup);
}

DarknetBaseNode::sendPacket(pkg, destAddr, destPort);
}
