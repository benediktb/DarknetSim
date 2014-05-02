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

#ifndef CHURNCONTROLLER_H_
#define CHURNCONTROLLER_H_

#include <csimplemodule.h>
#include "IRandomDistribution.h"
#include "ChurnMessage_m.h"
#include <LifecycleController.h>
#include <NodeOperations.h>
#include "./Debug.h"

typedef struct {
    std::string* nodeID;
    bool startState;
    std::vector<int>* switchTimes;
    unsigned int position;
} NodeTrace;

class DarknetChurnNode;

/**
 * Can be used with pre-defined traces. Trace file format is as follows (one
 * node per line):
 * <node ID>;<start state 0/1 == OFF/ON>;<comma-separated list of integer
 * ON/OFF durations>
 */
class ChurnController: public cSimpleModule {
private:
    bool useChurn;
    bool useTraces;

    /** Maps nodeID to their respective trace */
    std::map<std::string, NodeTrace*> nodeTraces;

    std::set<ChurnMessage*> pendingChurnMessages;

    virtual std::vector<std::string>* readNodesFromTraceFile(
            std::ifstream& tracefile);
    virtual void parseNodeLineFromTraceFile(
            std::map<std::string, NodeTrace*>& nodeTraces, std::string line);
    virtual void parseTraceFile(std::string filename);

    virtual NodeTrace* getTrace(std::string nodeID);
    virtual int getNextTraceSwitchTime(NodeTrace* trace);

    virtual void doStartupWithTraces(DarknetChurnNode* node);

protected:
    virtual void initialize();

    virtual void handleMessage(cMessage *msg);
    virtual void handleChurnMessage(ChurnMessage* cmsg);

    virtual void scheduleChurn(DarknetChurnNode* node, ChurnMessageType type,
            IRandomDistribution* distribution);
    virtual void scheduleChurn(DarknetChurnNode* node, ChurnMessageType type,
            double time);

public:
    ChurnController() :
            cSimpleModule::cSimpleModule(), useTraces(false) {
    }

    virtual ~ChurnController() {
        std::set<ChurnMessage*>::iterator it;
        for (it = pendingChurnMessages.begin();
                it != pendingChurnMessages.end(); it++) {
            cancelAndDelete(*it);
        }
    }

    virtual void doStartup(DarknetChurnNode* node);
};

#endif /* CHURNCONTROLLER_H_ */
