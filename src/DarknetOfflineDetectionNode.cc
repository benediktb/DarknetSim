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
    return DarknetSimpleNode::startApp(doneCallback);
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
    return DarknetSimpleNode::crashApp(doneCallback);
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
        if (friendsByID.find(msg->getSrcNodeID()) != friendsByID.end()) {
            EV<< "Received CON_SYN from: " << msg->getSrcNodeID() << endl;
            DarknetMessage *ack = new DarknetMessage("CON_SYNACK");
            ack->setType(DM_CON_SYNACK);
            ack->setTTL(defaultTTL);
            ack->setDestNodeID(msg->getSrcNodeID());
            sendDirectMessage(ack);
        }
        delete msg;
        break;
    }
    case DM_CON_SYNACK: {
        if (friendsByID.find(msg->getSrcNodeID()) != friendsByID.end()) {
            EV<< "Received CON_SYNACK from: " << msg->getSrcNodeID() << endl;
            DarknetMessage *ack = new DarknetMessage("CON_ACK");
            ack->setType(DM_CON_ACK);
            ack->setTTL(defaultTTL);
            ack->setDestNodeID(msg->getSrcNodeID());
            sendDirectMessage(ack);
            addActivePeer(msg->getSrcNodeID());
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
    connected.insert(nodeId);
    EV<< "Connection to " << nodeId << " established" << endl;
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
        DarknetSimpleNode::handleDarknetMessage(msg, sender);
    }
}

void DarknetOfflineDetectionNode::handleRcvAck(DarknetMessage* msg) {
    long orig_mID = msg->getRequestMessageID();
    if (rcvack_waiting.count(orig_mID) == 1) {
        DarknetMessage* msgPendingAck = rcvack_waiting.at(orig_mID).first;
        DarknetMessageType type = msgPendingAck->getType();
        long msgId = msgPendingAck->getId();
        EV<< "Received RCVACK for message: " << type << " (id:" << msgId << ")" << endl;
        cancelAndDelete(msgPendingAck);
        rcvack_waiting.erase(orig_mID);
    }
    delete msg;
}

void DarknetOfflineDetectionNode::sendRcvAck(DarknetMessage* msg) {
    EV<< "Send RCVACK for message: " << msg->getType() << " (ID:" << msg->getId()
    << "/treeID: " << msg->getTreeId() << ")" << endl;
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
    if (dm != NULL and dm->hasPar("origMsgID")
            and rcvack_waiting.count(dm->par("origMsgID").longValue()) == 1) {
        long msgID = dm->par("origMsgID").longValue();
        std::pair<DarknetMessage*, int>* waiting = &rcvack_waiting[msgID];

        if (waiting->second < resendCounter) {
            int destPort = (int) dm->par("destPort").longValue();
            IPvXAddress* destAddr =
                    (IPvXAddress*) (dm->par("destAddr").pointerValue());
            DarknetMessage* dup = dm->dup();
            DarknetSimpleNode::sendPacket(dup, *destAddr, destPort);
            emit(sigRetransmissionAfterTimeout,
                    dm->par("origMsgID").longValue());

            waiting->second++;
            waiting->first->par("origMsgID").setLongValue(dup->getId());
            rcvack_waiting.insert(std::make_pair(dup->getId(), *waiting));
            rcvack_waiting.erase(msgID);
            scheduleAt(simTime() + normal(resendTimerMean, resendTimerVariance),
                    dm);
        } else {
            /* too many resends; delete resendTimer and remove peer from the connected list */
            EV<< "Stop resendTimer for message: " << msg << " and remove the peer" << endl;
            emit(sigDropResendExeeded, waiting->first->getTTL());
            removeInactivePeer(dm->getDestNodeID());
            rcvack_waiting.erase(msgID);

            delete dm;
        }
    } else DarknetSimpleNode::handleSelfMessage(msg);
}

void DarknetOfflineDetectionNode::sendPacket(DarknetMessage* pkg,
        IPvXAddress& destAddr, int destPort) {
    // No ACKs for ACKs ... therefore no retransmissions
    if (pkg->getType() != DM_RCVACK) {
        DarknetMessage* dup = pkg->dup();

        std::stringstream nameStr;
        nameStr << "Retransmission timeout of "
                << DarknetMessage::typeToString(pkg->getType()) << " #"
                << pkg->getId();
        dup->setName(nameStr.str().c_str());

        dup->addPar("origMsgID");
        dup->par("origMsgID").setLongValue(pkg->getId());
        dup->addPar("destAddr");
        dup->addPar("destPort");
        dup->par("destAddr").setPointerValue(&destAddr);
        dup->par("destPort").setLongValue(destPort);

        EV<< "Start retransmission timer for message: (id:" << pkg->getId()
        << ", DestID: " << pkg->getDestNodeID() << ")" << endl;
        rcvack_waiting.insert(
                std::make_pair(pkg->getId(), std::make_pair(dup, (int) 0)));

        // Minimum reschedule time 10ms
        double rescheduleTime = std::max(
                normal(resendTimerMean, resendTimerVariance), 0.01);

        scheduleAt(simTime() + rescheduleTime, dup);
    }

    DarknetSimpleNode::sendPacket(pkg, destAddr, destPort);
}
