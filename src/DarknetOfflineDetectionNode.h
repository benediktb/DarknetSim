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

#ifndef DARKNETOFFLINEDETECTIONNODE_H_
#define DARKNETOFFLINEDETECTIONNODE_H_

#include "DarknetSimpleNode.h"

class DarknetOfflineDetectionNode: public DarknetSimpleNode {
public:
    DarknetOfflineDetectionNode() :
            DarknetSimpleNode::DarknetSimpleNode(), resendTimerMean(0), resendTimerVariance(
                    0), resendCounter(0), rcvack_waiting() {
    }
    ;
    virtual ~DarknetOfflineDetectionNode() {
        cancelAllRetransmissions();
    }
    ;

protected:
    double resendTimerMean;
    double resendTimerVariance;
    int resendCounter;
    std::map<long, std::pair<DarknetMessage*, int> > rcvack_waiting;

    simsignal_t sigRetransmissionAfterTimeout;
    simsignal_t sigDropResendExeeded;

    virtual void initialize(int stage);
    virtual void handleDarknetMessage(DarknetMessage* msg, DarknetPeer *sender);
    virtual void handleRcvAck(DarknetMessage* msg);
    virtual void sendRcvAck(DarknetMessage* msg);
    virtual void connectPeer(std::string nodeID);
    virtual void handleIncomingMessage(DarknetMessage* msg,
            DarknetPeer *sender);
    virtual void handleSelfMessage(cMessage* msg);
    virtual void sendPacket(DarknetMessage* pkg, IPvXAddress& destAddr,
            int destPort);

    virtual bool startApp(IDoneCallback *doneCallback);
    virtual bool stopApp(IDoneCallback *doneCallback);
    virtual bool crashApp(IDoneCallback *doneCallback);

    /* To extend/override */
    virtual void addActivePeer(std::string nodeId);
    virtual void removeInactivePeer(std::string peerId);

private:
    void cancelAllRetransmissions();
};

#endif /* DARKNETOFFLINEDETECTIONNODE_H_ */
