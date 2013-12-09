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

#ifndef DARKNETBASENODE_H_
#define DARKNETBASENODE_H_

#include <omnetpp.h>
#include <AppBase.h>
#include <IPvXAddress.h>
#include <UDPSocket.h>
#include "darknetmessage_m.h"

typedef std::pair<IPvXAddress, int> UDPAddress;

typedef struct {
    std::string nodeID;
    UDPAddress address;
} DarknetPeer;

typedef struct {
    long packetId;
    simtime_t eventTime;
    std::string srcId;
    std::string destId;
} seenPacket;

class DarknetBaseNode : public AppBase  {
public:
    DarknetBaseNode() {};
    virtual ~DarknetBaseNode() { };

protected:

    std::string nodeID;
    UDPSocket socket;
    int localPort;
    int defaultTTL;
    std::map<std::string, DarknetPeer*> peers;
    std::map<UDPAddress, DarknetPeer*> peersByAddress;
    std::set<std::string> connected;
    std::map<long, std::string > forwardedIdTable; // map for forwarded MessageIDs -> source nodeID
    std::set<long> outstandingResponses; // list for responses we are waiting for

    simsignal_t sigSendDM;
    simsignal_t sigUnhandledMSG;
    simsignal_t sigDropTtlExeeded;
    simsignal_t sigRequestRemainingTTL;
    simsignal_t sigResponseRemainingTTL;

    //things you probably don't have to change
    virtual int numInitStages() const { return 5; }
    virtual void addPeer(std::string nodeID, IPvXAddress& destAddr, int destPort);
    virtual void sendPacket(DarknetMessage* pkg, IPvXAddress& destAddr, int destPort);
    virtual bool sendDirectMessage(DarknetMessage* msg);
    virtual bool sendMessage(DarknetMessage* msg);
    virtual void handleUDPMessage(cMessage* msg);
    virtual DarknetMessage* makeRequest(std::string nodeID);
    virtual DarknetMessage* makeRequest(DarknetMessage *msg, std::string nodeID);
    virtual void sendToUDP(DarknetMessage *msg, int srcPort, const IPvXAddress& destAddr, int destPort);
    virtual void handleMessageWhenUp(cMessage *msg);


    //things you probably want to implement or extend
    virtual void initialize(int stage);
    virtual bool startApp(IDoneCallback *doneCallback) {return true;}
    virtual bool stopApp(IDoneCallback *doneCallback) {return true;}
    virtual bool crashApp(IDoneCallback *doneCallback) {return true;}
    virtual void handleDarknetMessage(DarknetMessage* msg, DarknetPeer *sender);
    virtual void handleIncomingMessage(DarknetMessage* msg, DarknetPeer *sender);
    virtual void forwardMessage(DarknetMessage* msg, DarknetPeer *sender);
    virtual void forwardResponse(DarknetMessage* msg);
    virtual void handleRequest(DarknetMessage* msg, DarknetPeer *sender);
    virtual bool canMessageBeForwarded(DarknetMessage* msg);
    virtual void doForwardingChangesOnMessage(DarknetMessage* msg);



    //things you have to implement
    virtual void connectPeer(std::string nodeID) = 0;
    virtual std::vector<DarknetPeer*> findNextHop(DarknetMessage* msg) = 0;
    virtual void handleSelfMessage(cMessage* msg) = 0;


};

#endif /* DARKNETBASENODE_H_ */
