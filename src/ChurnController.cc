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

#include "ChurnController.h"
#include "DarknetChurnNode.h"
#include <iostream>
#include <fstream>

Define_Module(ChurnController)
;

void ChurnController::initialize() {
    useChurn = par("useChurn").boolValue();

    if (!useChurn) {
        DEBUG("Not using churn" << endl);
        return;
    }

    useTraces = par("useTraces").boolValue();

    if (useTraces) {
        parseTraceFile(par("traceFile").stdstringValue());
    }

}

std::vector<std::string>* ChurnController::readNodesFromTraceFile(
        std::ifstream& tracefile) {
    std::vector<std::string>* nodeStrings = new std::vector<std::string>();

    if (tracefile.is_open()) {
        std::string line;
        while (getline(tracefile, line)) {
            nodeStrings->push_back(line);
        }
        tracefile.close();
    } else {
        return NULL;
    }

    return nodeStrings;
}

void ChurnController::parseNodeLineFromTraceFile(
        std::map<std::string, NodeTrace*>& nodeTraces, std::string line) {
    std::vector<std::string> lineParts =
            cStringTokenizer(line.c_str(), ";").asVector();

    if (lineParts.size() != 3) {
        error(("Malformed line in trace file: `" + line + "'").c_str());
    }

    NodeTrace* trace = new NodeTrace();

    trace->nodeID = new std::string(lineParts[0]);

    std::string stateString = lineParts[1];
    trace->startState = (stateString == "1");

    std::vector<int> switchTimesVec = cStringTokenizer(lineParts[2].c_str(),
            ",").asIntVector();
    // Make cleanly allocated copy
    std::vector<int>* switchTimes = new std::vector<int>(switchTimesVec);
    trace->switchTimes = switchTimes;

    trace->position = 0;
    nodeTraces.insert(make_pair(*trace->nodeID, trace));
}

void ChurnController::parseTraceFile(std::string filename) {
    DEBUG("Using trace file `" << filename << "' for churn data" << endl);

    std::ifstream tracefile(filename.c_str());
    std::vector<std::string>* nodeStrings = readNodesFromTraceFile(tracefile);

    if (nodeStrings == NULL) {
        error(
                ("Unable to open trace file. Maybe wrong working directory: `"
                        + std::string(getcwd(NULL, 0)) + "'").c_str());
    }

    nodeTraces = std::map<std::string, NodeTrace*>();

    std::vector<std::string>::iterator nIt;
    for (nIt = nodeStrings->begin(); nIt != nodeStrings->end(); nIt++) {
        parseNodeLineFromTraceFile(nodeTraces, *nIt);
    }

    DEBUG("Loaded " << nodeTraces.size() << " traces from file" << endl);
    delete nodeStrings;
}

NodeTrace* ChurnController::getTrace(std::string nodeID) {
    std::map<std::string, NodeTrace*>::iterator it;
    it = nodeTraces.find(nodeID);
    if (it == nodeTraces.end()) {
        return NULL;
    } else {
        return it->second;
    }
}

void ChurnController::doStartup(DarknetChurnNode* node) {
    Enter_Method_Silent
    (); // public method, possible context change

    if (!useChurn) {
        return;
    }

    if (useTraces) {
        doStartupWithTraces(node);
        return;
    }

    if (!node->startState) {
        DEBUG(
                "Node " << node->getNodeID() << " is OFF at the start, scheduling ON" << endl);
        scheduleChurn(node, CHURN_GO_ON, node->offTimeDistribution);
    } else {
        DEBUG(
                "Node " << node->getNodeID() << " is ON at the start, scheduling OFF" << endl);
        scheduleChurn(node, CHURN_GO_OFF, node->onTimeDistribution);
    }
}

int ChurnController::getNextTraceSwitchTime(NodeTrace* trace) {
    if (trace == NULL) {
        return -1;
    }
    if (trace->switchTimes->size() > trace->position) {
        return trace->switchTimes->at(trace->position++);
    } else {
        return -1;
    }
}

void ChurnController::doStartupWithTraces(DarknetChurnNode* node) {
    NodeTrace* trace = getTrace(node->getNodeID());

    int nextSwitchTime = getNextTraceSwitchTime(trace);

    if (nextSwitchTime == -1) {
        DEBUG(
                "Node " << node->getNodeID() << " is OFF at the start, won't go on at all " << "-OR- trace for this node is missing" << endl);
        node->startState = false;
        return;
    }

    node->startState = trace->startState;
    if (!trace->startState) {
        DEBUG(
                "Node " << node->getNodeID() << " is OFF at the start, scheduling ON" << endl);
        scheduleChurn(node, CHURN_GO_ON, nextSwitchTime);
    } else {
        DEBUG(
                "Node " << node->getNodeID() << " is ON at the start, scheduling OFF" << endl);
        scheduleChurn(node, CHURN_GO_OFF, nextSwitchTime);
    }
}

void ChurnController::handleMessage(cMessage* msg) {
    if (!msg->isSelfMessage()) {
        delete msg;
        error("ChurnController received unexpected non-selfmessage");
    }

    ChurnMessage* cmsg = dynamic_cast<ChurnMessage*>(msg);
    if (cmsg != NULL) {
        handleChurnMessage(cmsg);
        return;
    }

    delete msg;
    error("ChurnController received selfmessage of unknown type");
}

void ChurnController::handleChurnMessage(ChurnMessage* cmsg) {
    DarknetChurnNode* node = cmsg->getNode();

    int nextSwitchTime;

    if (useTraces) {
        NodeTrace* trace = getTrace(node->getNodeID());
        nextSwitchTime = getNextTraceSwitchTime(trace);

        if (nextSwitchTime == -1) {
            DEBUG(
                    "No more trace data for node " << node->getNodeID() << ", no more churn scheduled here" << endl);
        }
    }

    if (cmsg->getType() == CHURN_GO_ON) {
        node->churnGoOnline();

        if (!useTraces) {
            scheduleChurn(node, CHURN_GO_OFF, node->onTimeDistribution);
        } else if (nextSwitchTime > -1) {
            scheduleChurn(node, CHURN_GO_OFF, (double) nextSwitchTime);
        }
    } else {
        node->churnGoOffline();

        if (!useTraces) {
            scheduleChurn(node, CHURN_GO_ON, node->offTimeDistribution);
        } else if (nextSwitchTime > -1) {
            scheduleChurn(node, CHURN_GO_ON, (double) nextSwitchTime);
        }
    }

    pendingChurnMessages.erase(cmsg);
    delete cmsg;
}

void ChurnController::scheduleChurn(DarknetChurnNode* node,
        ChurnMessageType type, IRandomDistribution* distribution) {
    // Distribution result is in minutes, we schedule for seconds
    // Don't allow for 0 sec of ON/OFF time, minimum is 1 sec
    double nextChurnTime = (distribution->getNext() * 60.0) + 1.0;
    scheduleChurn(node, type, nextChurnTime);
}

void ChurnController::scheduleChurn(DarknetChurnNode* node,
        ChurnMessageType type, double time) {
    ChurnMessage* cmsg = new ChurnMessage(
            CS(node->getNodeID() << " " << ChurnMessageTypeToString(type)));

            // blame the OMNeT code formatter for this indentation!

            cmsg->setType(type);
            cmsg->setNode(node);

            // Allow max. churn time of 604800 sec == 1 week
            double limitedTime = std::min(time, 604800.0);

            // Make sure there is no overflow (of any kind)
            double curTimeD = simTime().dbl();
            double targetTime = curTimeD + limitedTime;
            double maxTime = SimTime::getMaxTime().dbl();
            if ((targetTime < curTimeD) or (targetTime >= maxTime)) {
                // If we are that close ... well, let it crash.
                double maxDelta = std::max(maxTime - curTimeD, 1.0);
                targetTime = curTimeD + 0.5 * maxDelta;
            }
            simtime_t scheduleTime = SimTime(targetTime);

            DEBUG(
                    "Scheduling churn type " << ChurnMessageTypeToString(type) << " on node " << node->getNodeID() << " in " << targetTime << endl);
    scheduleAt(scheduleTime, cmsg);
    pendingChurnMessages.insert(cmsg);
}
