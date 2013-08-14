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

#include "DarknetSimpleNode.h"

void DarknetSimpleNode::initialize(int stage) {
    DarknetBaseNode::initialize(stage);

    if (stage == 5) {
        std::vector<std::string> v = cStringTokenizer(par("pingID")).asVector();
        for(std::vector<std::string>::iterator iter = v.begin(); iter != v.end(); iter++) {
            cMessage *timer = new PingTimer((*iter));
                scheduleAt(1.0, timer);
        }
    }
}

void DarknetSimpleNode::connectPeer(std::string nodeID) {
    DarknetMessage *dm = new DarknetMessage();
    dm->setType(DM_CON_SYN);
    dm->setDestNodeID(nodeID.c_str());
    sendDirectMessage(dm);
};


void DarknetSimpleNode::handleSelfMessage(cMessage *msg) {
    if(dynamic_cast<PingTimer*>(msg) != NULL) {
        EV << "sending PING to: " << msg->getName();
        sendMessage(makeRequest(msg->getName()));
    }
    delete msg;
}

void DarknetSimpleNode::handleIncomingMessage(DarknetMessage *msg) {
    switch(msg->getType()) {
    case DM_CON_SYN: {
        if(peers.find(msg->getSrcNodeID()) != peers.end()) {
            EV << "recieved CON_SYN from: " << msg->getSrcNodeID() << endl;
            DarknetMessage *ack = new DarknetMessage();
            ack->setType(DM_CON_ACK);
            ack->setDestNodeID(msg->getSrcNodeID());
            sendDirectMessage(ack);
        }
        delete msg;
        break;
    }
    case DM_CON_ACK: {
        DarknetConnection *dc = new DarknetConnection;
        dc->nodeID = msg->getSrcNodeID();
        dc->lastSeen=0; //TODO fix
        connections.insert(std::pair<std::string,DarknetConnection*>(msg->getSrcNodeID(),dc));
        EV << "connection to " << dc->nodeID << "established" << endl;
        delete msg;
        break;
    }
    default:
        DarknetBaseNode::handleIncomingMessage(msg);
       break;
    }
}


