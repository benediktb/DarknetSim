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

Define_Module(ChurnController);

void ChurnController::initialize() {
    useTraces = par("useTraces").boolValue();

    if (useTraces) {
        EV << "Using trace file for churn" << endl;

        typedef std::vector<std::string> SV;
        typedef SV::iterator svIt;

        std::ifstream tracefile(par("traceFile").stringValue());
        SV nodeStrings;

        if (tracefile.is_open()) {
            std::string line;
            while (getline(tracefile, line)) {
                nodeStrings.push_back(line);
            }
            tracefile.close();
        } else {
            error(("Unable to open trace file. Maybe wrong working directory: " +
                    std::string(getcwd(NULL, 0))).c_str());
        }

        nodeTraces = std::map<std::string, NodeTrace*>();

        for (svIt nIt = nodeStrings.begin(); nIt != nodeStrings.end(); nIt++) {
            SV lineParts = cStringTokenizer(nIt->c_str(), ";").asVector();

            if (lineParts.size() != 3) {
                error(("Malformed line in trace file: " + *nIt).c_str());
            }

            NodeTrace* trace = new NodeTrace();

            trace->nodeID = new std::string(lineParts[0]);
            std::string stateString = lineParts[1];
            trace->startState = (stateString == "1");

            std::vector<int> switchTimesVec = cStringTokenizer(lineParts[2].c_str(), ",").asIntVector();
            // Make cleanly allocated copy
            std::vector<int>* switchTimes = new std::vector<int>(switchTimesVec);

            trace->switchTimes = switchTimes;
            trace->position = 0;
            nodeTraces.insert(make_pair(*trace->nodeID, trace));
         }

        EV << "Loaded " << nodeTraces.size() << " traces from file" << endl;
    }

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

void ChurnController::doStartup(cSimpleModule* node) {
    Enter_Method_Silent(); // public method, possible context change

    DarknetChurnNode* cnode = dynamic_cast<DarknetChurnNode*>(node);

    if (useTraces) {
        doStartupWithTraces(node);
        return;
    }

    if (!cnode->getStartState()) {
        EV << "Node " << cnode->getNodeID() << " is OFF at the start, scheduling ON" << endl;
        initShutdown(cnode->getParentModule());
        scheduleChurn(cnode, CHURN_GO_ON, cnode->getOffTimeDistribution());
    } else {
        EV << "Node " << cnode->getNodeID() << " is ON at the start, scheduling OFF" << endl;
        scheduleChurn(cnode, CHURN_GO_OFF, cnode->getOnTimeDistribution());
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

void ChurnController::doStartupWithTraces(cSimpleModule* node) {
    DarknetChurnNode* cnode = dynamic_cast<DarknetChurnNode*>(node);
    NodeTrace* trace = getTrace(cnode->getNodeID());

    int nextSwitchTime = getNextTraceSwitchTime(trace);

    if (nextSwitchTime == -1) {
        EV << "Node " << cnode->getNodeID() << " is OFF at the start, won't go on at all " <<
                "-OR- trace for this node is missing" << endl;
        initShutdown(cnode->getParentModule());
        return;
    }

    if (!trace->startState) {
        EV << "Node " << cnode->getNodeID() << " is OFF at the start, scheduling ON" << endl;
        cnode->setGoOnline(false);
        initShutdown(cnode->getParentModule());
        scheduleChurn(cnode, CHURN_GO_ON, nextSwitchTime);
    } else {
        EV << "Node " << cnode->getNodeID() << " is ON at the start, scheduling OFF" << endl;
        scheduleChurn(cnode, CHURN_GO_OFF, nextSwitchTime);
   }
}

void ChurnController::initOperation(cModule* parentModule, NodeOperation* operation) {
    std::map<std::string,std::string>* params = new std::map<std::string,std::string>();
    operation->initialize(parentModule, *params);

    lifecycleController->initiateOperation(operation, NULL);
}

void ChurnController::initStart(cModule* parentModule) {
    NodeStartOperation* op = new NodeStartOperation();
    initOperation(parentModule, op);
}

void ChurnController::initShutdown(cModule* parentModule) {
    NodeShutdownOperation* op = new NodeShutdownOperation();
    initOperation(parentModule, op);
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
    DarknetChurnNode* node = dynamic_cast<DarknetChurnNode*>(cmsg->getNode());

    int nextSwitchTime;

    if (useTraces) {
        NodeTrace* trace = getTrace(node->getNodeID());
        nextSwitchTime = getNextTraceSwitchTime(trace);

        if (nextSwitchTime == -1) {
            EV << "No more trace data for node " << node->getNodeID() <<
                    ", no more churn scheduled here" << endl;
        }
    }

    if (cmsg->getType() == CHURN_GO_ON) {
        node->setGoOnline(true);
        initStart(node->getParentModule());

        if (!useTraces) {
            scheduleChurn(node, CHURN_GO_OFF, node->getOnTimeDistribution());
        } else if (nextSwitchTime > -1) {
            scheduleChurn(node, CHURN_GO_OFF, nextSwitchTime);
        }
    } else {
        node->setGoOnline(false);
        initShutdown(node->getParentModule());

        if (!useTraces) {
            scheduleChurn(node, CHURN_GO_ON, node->getOffTimeDistribution());
        } else if (nextSwitchTime > -1) {
            scheduleChurn(node, CHURN_GO_ON, nextSwitchTime);
        }
    }
    delete cmsg;
}

void ChurnController::scheduleChurn(cSimpleModule* node,
        ChurnMessageType type, IRandomDistribution* distribution) {
    int nextChurnTime = (int) distribution->getNext();
    scheduleChurn(node, type, nextChurnTime);
}


void ChurnController::scheduleChurn(cSimpleModule* node,
        ChurnMessageType type, int time) {
    DarknetChurnNode* churnNode = dynamic_cast<DarknetChurnNode*>(node);
    ChurnMessage* cmsg = new ChurnMessage();

    cmsg->setType(type);
    cmsg->setNode(node);

    EV << "Scheduling churn type " << type << " on node " << churnNode->getNodeID() << " in " << time << endl;
    scheduleAt(simTime() + time, cmsg);
}
