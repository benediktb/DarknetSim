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

Define_Module(DarknetChurnNode)
;

void DarknetChurnNode::initialize(int stage) {
    DarknetOfflineDetectionNode::initialize(stage);

    if (stage == 0) {
        sigChurnOnOff = registerSignal("sigChurnOnOff");
        sigChurnOff = registerSignal("sigChurnOff");
        sigChurnOn = registerSignal("sigChurnOn");

        startState = par("startState").boolValue();
        usePings = par("usePings").boolValue();
        pingFrequency = par("pingFrequency");

        churnController =
                dynamic_cast<ChurnController *>(simulation.getModuleByPath("churnController"));

        if (churnController == NULL) {
            error(
                    "Could not find ChurnController. Is there one in the network (with name 'churnController')?");
        }

        // Setup online time distribution
        std::string onDist = par("onTimeDistribution").stringValue();
        onTimeDistribution = RandomDistributionFactory::getDistribution(onDist,
                this, "onTime");
        if (onTimeDistribution == NULL) {
            error(("Unknown random distribution: " + onDist).c_str());
        }

        // Setup offline time distribution
        std::string offDist = par("offTimeDistribution").stringValue();
        offTimeDistribution = RandomDistributionFactory::getDistribution(
                offDist, this, "offTime");
        if (offTimeDistribution == NULL) {
            error(("Unknown random distribution: " + offDist).c_str());
        }

        /* Setup churn controller, i.e. schedule churn events and set online
         * state if needed */
        churnController->doStartup(this);
    }
}

void DarknetChurnNode::handleSelfMessage(cMessage* msg) {
    PingMessage* pmsg = dynamic_cast<PingMessage*>(msg);
    if (usePings and (pmsg != NULL)) {
        DarknetMessage* ping = new DarknetMessage("DM_PING");
        ping->setType(DM_PING);
        ping->setDestNodeID(pmsg->getPeerId());
        sendDirectMessage(ping);
        // No re-scheduling here: If no ACK -> remove peer; on ACK arrival it is
        //  re-scheduled
    } else {
        DarknetOfflineDetectionNode::handleSelfMessage(msg);
    }
}
void DarknetChurnNode::handleDarknetMessage(DarknetMessage *msg,
        DarknetPeer *sender) {
    if (usePings) {
        // Reset ping timer
        std::map<std::string, PingMessage*>::iterator pmIt = pingMessages.find(
                sender->nodeID);
        if (pmIt != pingMessages.end()) {
            PingMessage* pmsg = pmIt->second;
            cancelEvent(pmsg);
            scheduleAt(calcNextPingTime(), pmsg);
        }
    }

    DarknetOfflineDetectionNode::handleDarknetMessage(msg, sender);
}

void DarknetChurnNode::handleIncomingMessage(DarknetMessage *msg,
        DarknetPeer *sender) {
    if (msg->getType() == DM_PING) {
        // We don't need this anymore, RCVACK is already sent
        delete msg;
    } else {
        DarknetOfflineDetectionNode::handleIncomingMessage(msg, sender);
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

void DarknetChurnNode::sendToUDP(DarknetMessage *msg, int srcPort,
        const IPvXAddress& destAddr, int destPort) {
    if (isOnline) {
        DarknetOfflineDetectionNode::sendToUDP(msg, srcPort, destAddr,
                destPort);
    } else {
        error("Tried to send UDP packet while offline.");
    }
}

simtime_t DarknetChurnNode::calcNextPingTime() {
    return simTime() + pingFrequency + uniform(-1, 1);
}

void DarknetChurnNode::addActivePeer(std::string nodeId) {
    DarknetOfflineDetectionNode::addActivePeer(nodeId);
    if (usePings) {
        PingMessage* pmsg = new PingMessage(("ping " + nodeId).c_str());
        pmsg->setPeerId(nodeId.c_str());
        pingMessages.insert(std::make_pair(nodeId, pmsg));
        scheduleAt(calcNextPingTime(), pmsg);
    }
}

void DarknetChurnNode::removeInactivePeer(std::string peerId) {
    if (usePings) {
        std::map<std::string, PingMessage*>::iterator pmIt = pingMessages.find(peerId);
        if (pmIt != pingMessages.end()) {
            cancelAndDelete(pmIt->second);
            pingMessages.erase(peerId);
        }
    }
    DarknetOfflineDetectionNode::removeInactivePeer(peerId);
}

void DarknetChurnNode::churnGoOnline() {
    Enter_Method_Silent
    (); // possible context change from churnController
    goOnline();
}

void DarknetChurnNode::goOnline() {
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

void DarknetChurnNode::churnGoOffline() {
    Enter_Method_Silent
    (); // possible context change from churnController
    goOffline();
}

void DarknetChurnNode::goOffline() {
    DarknetOfflineDetectionNode::crashApp(NULL);

    if (usePings) {
        std::map<std::string, PingMessage*>::iterator pmIt;
        for (pmIt = pingMessages.begin(); pmIt != pingMessages.end(); pmIt++) {
            cancelAndDelete(pmIt->second);
        }
        pingMessages.clear();
    }

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
    return crashApp(doneCallback);
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
    int nextHopIndex = intuniform(0, connected.size() - 1);
    std::vector<DarknetPeer*> list;
    int i = 0;
    for (std::set<std::string>::iterator it = connected.begin();
            it != connected.end(); it++) {
        if (i == nextHopIndex) {
            list.push_back(friendsByID[*it]);
            break;
        }
        i++;
    }
    return list;
}
