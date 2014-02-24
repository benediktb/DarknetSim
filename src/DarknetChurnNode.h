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

#ifndef DARKNETCHURNNODE_H_
#define DARKNETCHURNNODE_H_

#include "DarknetOfflineDetectionNode.h"
#include "ChurnController.h"
#include "ChurnMessage_m.h"
#include "IRandomDistribution.h"
#include <NodeOperations.h>

/**
 * Darknet node with churn support.
 *
 * Distributions for on/off-time: See distribution/ subfolder and the factory
 * class RandomDistributionFactory.
 *
 * Signals: Will be emitted when going on/offline, i.e. sigChurnOn collects
 * offline times when going online.
 */
class DarknetChurnNode: public DarknetOfflineDetectionNode {
    /* Allow direct access for ChurnController, so no public getters/setters are
     * necessary.
     */
    friend class ChurnController;

protected:
    simsignal_t sigChurnOnOff;
    simsignal_t sigChurnOn;
    simsignal_t sigChurnOff;

    ChurnController* churnController;
    bool startState;
    bool isOnline;
    simtime_t lastSwitch;

    /** Distribution for online time (time till next offline time) */
    IRandomDistribution* onTimeDistribution;

    /** Distribution for offline time (time till next online time) */
    IRandomDistribution* offTimeDistribution;

    virtual void sendToUDP(DarknetMessage *msg, int srcPort, const IPvXAddress& destAddr, int destPort);
    virtual void handleUDPMessage(cMessage* msg);

    virtual void markAsOffline();

    virtual bool startApp(IDoneCallback *doneCallback);
    virtual bool stopApp(IDoneCallback *doneCallback);
    virtual bool crashApp(IDoneCallback *doneCallback);

    // To extend/overwrite:
    virtual void initialize(int stage);

    virtual std::vector<DarknetPeer*> findNextHop(DarknetMessage* msg);

    void churnGoOnline();
    virtual void goOnline();
    void churnGoOffline();
    virtual void goOffline();

public:
    DarknetChurnNode() : DarknetOfflineDetectionNode::DarknetOfflineDetectionNode(), isOnline(false), lastSwitch(0) {};

    virtual ~DarknetChurnNode() {
        delete onTimeDistribution;
        delete offTimeDistribution;
    };

};

#endif /* DARKNETCHURNNODE_H_ */
