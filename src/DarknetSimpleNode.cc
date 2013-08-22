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
#include "algorithm"

void DarknetSimpleNode::initialize(int stage) {
    DarknetBaseNode::initialize(stage);

    if (stage == 5) {
        std::vector<std::string> v = cStringTokenizer(par("requestTargets")).asVector();
        double requestIntervalMean = par("requestIntervalMean");
        double requestIntervalVariance = par("requestIntervalVariance");
        simtime_t sendAt = simTime();
        for(std::vector<std::string>::iterator iter = v.begin(); iter != v.end(); iter++) {
            cMessage *timer = new PingTimer((*iter));
            sendAt += normal(requestIntervalMean,requestIntervalVariance);
            scheduleAt(sendAt, timer);
        }
    }
}

void DarknetSimpleNode::connectPeer(std::string nodeID) {
    if(connected.find(nodeID) != connected.end()) {
        connected.insert(nodeID);
    }
};


void DarknetSimpleNode::handleSelfMessage(cMessage *msg) {
    if(dynamic_cast<PingTimer*>(msg) != NULL) {
        EV << "sending PING to: " << msg->getName();
        sendMessage(makeRequest(msg->getName()));
    }
    delete msg;
}
