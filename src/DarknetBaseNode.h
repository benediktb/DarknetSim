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
#include <IUDPSocket.h>
#include "darknetmessage.h"
#include "MessageCallback.h"
#include "./Debug.h"

typedef std::pair<IPvXAddress, int> UDPAddress;

typedef struct DarknetPeer DarknetPeer;
struct DarknetPeer {
    std::string nodeID;
    UDPAddress address;
    bool connected;
    bool operator<(const DarknetPeer& other) const {
        return address < other.address;
    }
    ;
    bool operator==(const DarknetPeer& other) const {
        return nodeID == other.nodeID;
    }
    ;
};

typedef struct {
    long packetId;
    simtime_t eventTime;
    std::string srcId;
    std::string destId;
} seenPacket;

#define DARKNET_MESSAGE_ISEXTERNAL "darknet_msg_is_external"
#define DARKNET_MESSAGE_EXTERNAL_CALLBACK "darknet_msg_external_callback"

class DarknetBaseNode: public AppBase {
public:
    DarknetBaseNode() :
            socket(NULL) {
    }
    virtual ~DarknetBaseNode() {
        delete socket;
    }

    std::string getNodeID();

    virtual void handleExternalMessage(cMessage *msg, simtime_t& when,
            MessageCallback* callback);

protected:

    std::string nodeID;
    IUDPSocket* socket;
    int localPort;
    int defaultTTL;

    /* Friendlist, indexed by ID and IP address */
    std::map<std::string, DarknetPeer*> friendsByID;
    std::map<IPvXAddress, DarknetPeer*> friendsByAddress;

    /** Number of currently connected peers */
    long numConnected;

    /** Map for forwarded MessageIDs -> source nodeID */
    std::map<long, std::string> forwardedIdTable;

    /** List for responses we are waiting for */
    std::set<long> outstandingResponses;

    simsignal_t sigSendDM;
    simsignal_t sigUnhandledMSG;
    simsignal_t sigDropTtlExeeded;
    simsignal_t sigRequestRemainingTTL;
    simsignal_t sigResponseRemainingTTL;

    // Things you probably don't have to change
    virtual int numInitStages() const {
        return 5;
    }
    virtual IUDPSocket* getSocket();
    virtual void connectAllFriends();
    virtual void addPeer(std::string nodeID, IPvXAddress& destAddr,
            int destPort);

    virtual DarknetMessage* makeRequest(std::string nodeID);
    virtual DarknetMessage* makeRequest(DarknetMessage *msg,
            std::string nodeID);
    virtual void makeResponse(DarknetMessage *msg, DarknetMessage *request);
    virtual bool sendDirectMessage(DarknetMessage* msg);
    virtual bool sendMessage(DarknetMessage* msg);
    virtual void sendPacket(DarknetMessage* pkg, IPvXAddress& destAddr,
            int destPort);
    virtual void sendToUDP(DarknetMessage *msg, int srcPort,
            const IPvXAddress& destAddr, int destPort);
    virtual IPv4Address getLocalIPv4Address();

    virtual void handleMessageWhenUp(cMessage *msg);
    virtual void handleUDPMessage(cMessage* msg);

    // Things you probably want to implement or extend
    virtual void initialize(int stage);
    virtual bool startApp(IDoneCallback *doneCallback);
    virtual bool stopApp(IDoneCallback *doneCallback);
    virtual bool crashApp(IDoneCallback *doneCallback);

    virtual void handleDarknetMessage(DarknetMessage* msg, DarknetPeer *sender);
    virtual void handleIncomingMessage(DarknetMessage* msg,
            DarknetPeer *sender);
    virtual void handleRequest(DarknetMessage* msg, DarknetPeer *sender);
    virtual void forwardMessage(DarknetMessage* msg, DarknetPeer *sender);
    virtual void forwardResponse(DarknetMessage* msg);

    // Things you have to implement
    virtual void connectPeer(std::string nodeID) = 0;

    virtual void handleSelfMessage(cMessage* msg) = 0;
    virtual std::vector<DarknetPeer*> findNextHop(DarknetMessage* msg) = 0;

};

#endif /* DARKNETBASENODE_H_ */
