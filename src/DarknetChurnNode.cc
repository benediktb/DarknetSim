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

void DarknetChurnNode::setGoOnline(bool goOnline) {
    this->goOnline = goOnline;
}

bool DarknetChurnNode::getStartState() const {
    return par("startState").boolValue();
}

IRandomDistribution* DarknetChurnNode::getOnTimeDistribution() const {
    return onTimeDistribution;
}

IRandomDistribution* DarknetChurnNode::getOffTimeDistribution() const {
    return offTimeDistribution;
}

void DarknetChurnNode::initialize(int stage) {
    switch(stage) {
    case 0: {
        sigChurnOnOff = registerSignal("sigChurnOnOff");
        sigChurnOff = registerSignal("sigChurnOff");
        sigChurnOn = registerSignal("sigChurnOn");

        churnController = dynamic_cast<ChurnController *>(simulation.getModuleByPath("churnController"));

        if (churnController == NULL) {
            error("Could not find ChurnController. Is there one in the network (with name 'churnController')?");
        }

        /* Check whether to start at all */
        goOnline = par("startState").boolValue();

        /* Setup online time distribution */
        std::string onDist = par("onTimeDistribution").stringValue();
        onTimeDistribution = RandomDistributionFactory::getDistribution(onDist, this, "onTime");
        if (onTimeDistribution == NULL) {
            error(("Unknown random distribution: " + onDist).c_str());
        }

        /* Setup offline time distribution */
        std::string offDist = par("offTimeDistribution").stringValue();
        offTimeDistribution = RandomDistributionFactory::getDistribution(offDist, this, "offTime");
        if (offTimeDistribution == NULL) {
            error(("Unknown random distribution: " + offDist).c_str());
        }

        break;
    }
    case 3:
        /* If not started at the beginning: Stop app again (is automatically
         * started in stage 3 by AppBase) before connections are built up.
         */
        churnController->doStartup(this);
        break;
    }

    /* Run super class initialization until socket binding (clashes with
     *   offline nodes, therefore moved to startApp() )
     */
    DarknetOfflineDetectionNode::initialize(stage);
}

bool DarknetChurnNode::startApp(IDoneCallback *doneCallback) {
    if (goOnline) {
        DarknetOfflineDetectionNode::startApp(doneCallback);
        //connectAllFriends(); // will be done by DarknetBaseNode::startApp()
        cDisplayString& dispStr = getParentModule()->getDisplayString();
        dispStr.updateWith("i=device/pc2,green");
        emit(sigChurnOn, simTime() - lastSwitch);
        emit(sigChurnOnOff, 1);
        lastSwitch = simTime();
    }
    return true;
}

bool DarknetChurnNode::stopApp(IDoneCallback *doneCallback) {
    return crashApp(doneCallback);
}

bool DarknetChurnNode::crashApp(IDoneCallback *doneCallback) {
    DarknetOfflineDetectionNode::crashApp(doneCallback);
    cDisplayString& dispStr = getParentModule()->getDisplayString();
    dispStr.updateWith("i=device/pc2,red");
    emit(sigChurnOff, simTime() - lastSwitch);
    emit(sigChurnOnOff, 0);
    lastSwitch = simTime();
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
