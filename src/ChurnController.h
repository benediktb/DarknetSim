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
 *
 * Note: cModule* == StandardHost*, parent of DarknetChurnNode* (usually, but
 * not necessarily, therefore a more common type is used here).
 */
class ChurnController: public cSimpleModule {
private:
    bool useChurn;

    LifecycleController* lifecycleController;
    bool useTraces;
    std::map<std::string, NodeTrace*> nodeTraces;

    NodeTrace* getTrace(std::string nodeID);
    int getNextTraceSwitchTime(NodeTrace* trace);

    void doStartupWithTraces(DarknetChurnNode* node);
    void initStart(cModule* parentModule);
    void initShutdown(cModule* parentModule);

protected:
    virtual void initialize();

    virtual void handleMessage(cMessage *msg);
    virtual void handleChurnMessage(ChurnMessage* cmsg);

    void initOperation(cModule* parentModule, NodeOperation* operation);
    virtual void scheduleChurn(DarknetChurnNode* node, ChurnMessageType type, IRandomDistribution* distribution);
    virtual void scheduleChurn(DarknetChurnNode* node, ChurnMessageType type, int time);

public:
    ChurnController(): cSimpleModule::cSimpleModule(), lifecycleController(new LifecycleController()), useTraces(false) {}
    ~ChurnController() {
        delete lifecycleController;
    }

    /**
     * Used at initialization by DarknetChurnNode. Will crash when used by
     * other node type (parameter "node" is casted to DarknetChurnNode*).
     */
    virtual void doStartup(DarknetChurnNode* node);
};

#endif /* CHURNCONTROLLER_H_ */
