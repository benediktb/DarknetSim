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
    if (stage == 4) {
        resendCounter = par("resendCounter");
        resendTimer = par("resendTimer");
    }
}

/*
 * check if message is of type DM_RCVACK, if so stop resendTimer for according message.
 * if not, send DM_RCVACK to sender and pass it to handleIncomingMessage
 */
void DarknetOfflineDetectionNode::handleDarknetMessage(DarknetMessage* msg) {
    if( msg->getType() == DM_RCVACK) {
        long orig_mID = msg->getRequestMessageID();
        if(rcvack_waiting.find(orig_mID) != rcvack_waiting.end()) {
            EV << "received RCVACK for message: " << rcvack_waiting[orig_mID].first->getType() << " (id:" <<rcvack_waiting[orig_mID].first->getId() << ")"  << endl;
            cancelAndDelete(rcvack_waiting[orig_mID].first);
            rcvack_waiting.erase(orig_mID);
        }
        delete msg;
    } else {
        EV << "send RCVACK for message: " << msg->getType() << " (ID:" << msg->getId() << "/treeID: " << msg->getTreeId() << ")"  << endl;
        DarknetMessage* ack = new DarknetMessage();
        ack->setDestNodeID(msg->getSrcNodeID());
        ack->setType(DM_RCVACK);
        ack->setRequestMessageID(msg->getId());
        sendDirectMessage(ack);
        DarknetSimpleNode::handleDarknetMessage(msg);
    }
}

/*
 * check if msg is a DarknetMessage in the list of sent messages waiting for an RCVACK.
 * if so, check if resendCounter is reached and otherwise resend (and reschedule it)
 */
void DarknetOfflineDetectionNode::handleSelfMessage(cMessage* msg) {
    DarknetMessage* dm = dynamic_cast<DarknetMessage*>(msg);
    if(dm != NULL and dm->hasPar("origMsgId") and  rcvack_waiting.find(dm->par("origMsgId").longValue()) != rcvack_waiting.end()) {
        long mID = dm->par("origMsgId").longValue();

        if(rcvack_waiting[mID].second < resendCounter) {
            int destPort = (int) dm->par("destPort").longValue();
            IPvXAddress* destAddr = (IPvXAddress*) (dm->par("destAddr").pointerValue());
            DarknetMessage* dup = dm->dup();
            DarknetSimpleNode::sendPacket(dup, *destAddr, destPort);
            rcvack_waiting[mID].second++;
            dm->par("origMsgID").setLongValue(dup->getId());
            rcvack_waiting.insert(std::pair<long, std::pair<DarknetMessage*,int> >(dup->getId(),rcvack_waiting[mID]));
            rcvack_waiting.erase(mID);
            scheduleAt(simTime() + resendTimer,dm);
        } else {
            EV << "stop resendTimer for message: " << msg;
            rcvack_waiting.erase(mID);
            delete dm;
        }
    } else DarknetSimpleNode::handleSelfMessage(msg);
}

/*
 * add resend timer for message; resend if no DM_RCVACK will be received
 */
//bool DarknetOfflineDetectionNode::sendMessage(DarknetMessage* msg) {
//    bool status = DarknetSimpleNode::sendMessage(msg->dup());
//    if(status) {
//        EV << "start resend timer for message: " << msg->getType() << " (id:" << msg->getTreeId() << ")"  << endl;
//        rcvack_waiting.insert(std::pair<long, std::pair<DarknetMessage*,int> >(msg->getTreeId(),std::pair<DarknetMessage*,int>(msg,0)));
//        scheduleAt(simTime() + resendTimer,msg);
//    } else delete msg; // could not send message - we can't do anything
//    return status;
//}


/*
 * add resend timer for message; resend if no DM_RCVACK will be received
 */

void DarknetOfflineDetectionNode::sendPacket(DarknetMessage* pkg, IPvXAddress& destAddr, int destPort) {

    DarknetMessage* dup = pkg->dup();
    dup->addPar("origMsgID");
    dup->par("origMsgID").setLongValue(pkg->getId());
    dup->addPar("destAddr");
    dup->addPar("destPort");
    dup->par("destAddr").setPointerValue(&destAddr);
    dup->par("destPort").setLongValue(destPort);
    EV << "start resend timer for message: (id:" << pkg->getId() << ", DestID: "<<pkg->getDestNodeID() << ")"  << endl;
    rcvack_waiting.insert(std::pair<long, std::pair<DarknetMessage*,int> >(pkg->getId(),std::pair<DarknetMessage*,int>(dup,0)));
    scheduleAt(simTime() + resendTimer,dup);

    DarknetSimpleNode::sendPacket(pkg, destAddr, destPort);
}
