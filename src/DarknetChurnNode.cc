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

#include "DarknetChurnNode.h"
#include "RandomDistributionFactory.h"
#include <csimulation.h>

Define_Module(DarknetChurnNode);

void DarknetChurnNode::initialize(int stage) {
    DarknetOfflineDetectionNode::initialize(stage);

    if (stage == 0) {
        sigChurnOnOff = registerSignal("sigChurnOnOff");
        sigChurnOff = registerSignal("sigChurnOff");
        sigChurnOn = registerSignal("sigChurnOn");

        startState = par("startState").boolValue();

        churnController = dynamic_cast<ChurnController *>(simulation.getModuleByPath("churnController"));

        if (churnController == NULL) {
            error("Could not find ChurnController. Is there one in the network (with name 'churnController')?");
        }

        // Setup online time distribution
        std::string onDist = par("onTimeDistribution").stringValue();
        onTimeDistribution = RandomDistributionFactory::getDistribution(onDist, this, "onTime");
        if (onTimeDistribution == NULL) {
            error(("Unknown random distribution: " + onDist).c_str());
        }

        // Setup offline time distribution
        std::string offDist = par("offTimeDistribution").stringValue();
        offTimeDistribution = RandomDistributionFactory::getDistribution(offDist, this, "offTime");
        if (offTimeDistribution == NULL) {
            error(("Unknown random distribution: " + offDist).c_str());
        }

        /* Setup churn controller, i.e. schedule churn events and set online
         * state if needed */
        churnController->doStartup(this);
    }
}

void DarknetChurnNode::handleUDPMessage(cMessage* msg) {
    if (isOnline) {
        DarknetOfflineDetectionNode::handleUDPMessage(msg);
    } else {
        // Received UDP packet while offline: Discard
        delete msg;
    }
}

void DarknetChurnNode::sendToUDP(DarknetMessage *msg, int srcPort, const IPvXAddress& destAddr, int destPort) {
    if (isOnline) {
        DarknetOfflineDetectionNode::sendToUDP(msg, srcPort, destAddr, destPort);
    } else {
        error("Tried to send UDP packet while offline.");
    }
}


void DarknetChurnNode::goOnline() {
    Enter_Method_Silent(); // possible context change from churnController
    cDisplayString& dispStr = getParentModule()->getDisplayString();
    dispStr.updateWith("i=device/pc2,green");
    emit(sigChurnOn, simTime() - lastSwitch);
    emit(sigChurnOnOff, 1);
    lastSwitch = simTime();
    isOnline = true;
    connectAllFriends();
}

void DarknetChurnNode::markAsOffline() {
    cDisplayString& dispStr = getParentModule()->getDisplayString();
    dispStr.updateWith("i=device/pc2,red");
}

void DarknetChurnNode::goOffline() {
    Enter_Method_Silent(); // possible context change from churnController
    DarknetOfflineDetectionNode::crashApp(NULL);
    markAsOffline();
    emit(sigChurnOff, simTime() - lastSwitch);
    emit(sigChurnOnOff, 0);
    lastSwitch = simTime();
    isOnline = false;
}

bool DarknetChurnNode::startApp(IDoneCallback *doneCallback) {
    if (startState) {
        goOnline();
    } else {
        markAsOffline();
    }
    DarknetOfflineDetectionNode::startApp(doneCallback);
    return true;
}

bool DarknetChurnNode::stopApp(IDoneCallback *doneCallback) {
    return DarknetOfflineDetectionNode::stopApp(doneCallback);
}

bool DarknetChurnNode::crashApp(IDoneCallback *doneCallback) {
    goOffline();
    //DarknetOfflineDetectionNode::crashApp(doneCallback);
    return true;
}

/**
 * Chooses random next hop.
 */
std::vector<DarknetPeer*> DarknetChurnNode::findNextHop(DarknetMessage* msg) {
    int nextHopIndex = uniform(0, 1) * connected.size();
    std::vector<DarknetPeer*> list;
    int i = 0;
    for(std::set<std::string>::iterator it = connected.begin(); it != connected.end(); it++) {
        if (i == nextHopIndex) {
            list.push_back(friendsByID[*it]);
            break;
        }
        i++;
    }
    return list;
}
