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

//#include <IPAddressResolver.h>
#include "RandomwalkNode.h"

Define_Module(RandomwalkNode)
;

void RandomwalkNode::initialize(int stage) {
    if (stage == 0) {
        requestFanout = par("requestFanout");
    }
//    DarknetOfflineDetectionNode::initialize(stage);
    DarknetSimpleNode::initialize(stage);
}

std::vector<DarknetPeer*> RandomwalkNode::findNextHop(DarknetMessage* msg) {
    if (!connected.size()) { // peer list empty -> raise exception?
        EV<< "ERROR: empty peer list!";
        return std::vector<DarknetPeer*>(0);
    }
    if(connected.count(msg->getDestNodeID()) == 1 and friendsByID.count(msg->getDestNodeID()) == 1) {
        return std::vector<DarknetPeer*>(1,friendsByID[msg->getDestNodeID()]);
    } else {
        std::set<std::string>::iterator iter = connected.begin();
        std::advance(iter, dblrand() * connected.size());
        return std::vector<DarknetPeer*>(1,friendsByID[*iter]);
    }
}

        /*
         * send <requestFanout> requests at once, each can travel a different path (since all have different treeIDs)
         * save the timers ID in RequestMessageID, so only the first request will be answered
         */
void RandomwalkNode::handleSelfMessage(cMessage *msg) {
    if (dynamic_cast<PingTimer*>(msg) != NULL) {
        EV<< "sending PING to: " << msg->getName();
        DarknetMessage* m;
        for(int i=0; i< requestFanout; i++) {
            m = makeRequest(msg->getName());
            m->setRequestMessageID(msg->getId());
            sendMessage(m);
        }
        delete msg;
//    }else DarknetOfflineDetectionNode::handleSelfMessage(msg);
    } else DarknetSimpleNode::handleSelfMessage(msg);
}

        /*
         * only respond to the first request with the same RequestMessageID
         */
void RandomwalkNode::handleRequest(DarknetMessage* request) {
    if (answeredRequests.count(request->getRequestMessageID()) == 0) {
        answeredRequests.insert(request->getRequestMessageID());
        DarknetMessage *msg = new DarknetMessage();
        msg->setDestNodeID(request->getSrcNodeID());
        msg->setType(DM_RESPONSE);
        msg->setTTL(defaultTTL - request->getTTL()); // TTL is set to the path length on the forward run
        msg->setRequestMessageID(request->getTreeId());
        delete request;
        sendMessage(msg);
    } else
        delete request;
}
