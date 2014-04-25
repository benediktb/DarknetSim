#include "IUDPSocket.h"

//
// Copyright (C) 2005,2011 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "UDPControlInfo.h"
#include "IPv4InterfaceData.h"

IUDPSocket::IUDPSocket()
{
    // don't allow user-specified sockIds because they may conflict with
    // automatically assigned ones.
    sockId = generateSocketId();
    gateToUdp = NULL;
}

int IUDPSocket::generateSocketId()
{
    return ev.getUniqueNumber();
}

void IUDPSocket::sendToUDP(cMessage *msg)
{
    if (!gateToUdp)
        throw cRuntimeError("IUDPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToUdp->getOwnerModule())->send(msg, gateToUdp);
}

void IUDPSocket::bind(int localPort)
{
    bind(IPvXAddress(), localPort);
}

void IUDPSocket::bind(IPvXAddress localAddr, int localPort)
{
    if (localPort<-1 || localPort>65535)  // -1: ephemeral port
        throw cRuntimeError("IUDPSocket::bind(): invalid port number %d", localPort);

    UDPBindCommand *ctrl = new UDPBindCommand();
    ctrl->setSockId(sockId);
    ctrl->setLocalAddr(localAddr);
    ctrl->setLocalPort(localPort);
    cMessage *msg = new cMessage("BIND", UDP_C_BIND);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::connect(IPvXAddress addr, int port)
{
    if (addr.isUnspecified())
        throw cRuntimeError("IUDPSocket::connect(): unspecified remote address");
    if (port<=0 || port>65535)
        throw cRuntimeError("IUDPSocket::connect(): invalid remote port number %d", port);

    UDPConnectCommand *ctrl = new UDPConnectCommand();
    ctrl->setSockId(sockId);
    ctrl->setRemoteAddr(addr);
    ctrl->setRemotePort(port);
    cMessage *msg = new cMessage("CONNECT", UDP_C_CONNECT);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::sendTo(cPacket *pk, IPvXAddress destAddr, int destPort)
{
    pk->setKind(UDP_C_DATA);
    UDPSendCommand *ctrl = new UDPSendCommand();
    ctrl->setSockId(sockId);
    ctrl->setDestAddr(destAddr);
    ctrl->setDestPort(destPort);
    pk->setControlInfo(ctrl);
    sendToUDP(pk);
}

void IUDPSocket::sendTo(cPacket *pk, IPvXAddress destAddr, int destPort, int outInterface)
{
    pk->setKind(UDP_C_DATA);
    UDPSendCommand *ctrl = new UDPSendCommand();
    ctrl->setSockId(sockId);
    ctrl->setDestAddr(destAddr);
    ctrl->setDestPort(destPort);
    ctrl->setInterfaceId(outInterface);
    pk->setControlInfo(ctrl);
    sendToUDP(pk);
}
void IUDPSocket::send(cPacket *pk)
{
    pk->setKind(UDP_C_DATA);
    UDPSendCommand *ctrl = new UDPSendCommand();
    ctrl->setSockId(sockId);
    pk->setControlInfo(ctrl);
    sendToUDP(pk);
}

void IUDPSocket::close()
{
    cMessage *msg = new cMessage("CLOSE", UDP_C_CLOSE);
    UDPCloseCommand *ctrl = new UDPCloseCommand();
    ctrl->setSockId(sockId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::setBroadcast(bool broadcast)
{
    cMessage *msg = new cMessage("SetBroadcast", UDP_C_SETOPTION);
    UDPSetBroadcastCommand *ctrl = new UDPSetBroadcastCommand();
    ctrl->setSockId(sockId);
    ctrl->setBroadcast(broadcast);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::setTimeToLive(int ttl)
{
    cMessage *msg = new cMessage("SetTTL", UDP_C_SETOPTION);
    UDPSetTimeToLiveCommand *ctrl = new UDPSetTimeToLiveCommand();
    ctrl->setSockId(sockId);
    ctrl->setTtl(ttl);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::setTypeOfService(unsigned char tos)
{
    cMessage *msg = new cMessage("SetTOS", UDP_C_SETOPTION);
    UDPSetTypeOfServiceCommand *ctrl = new UDPSetTypeOfServiceCommand();
    ctrl->setSockId(sockId);
    ctrl->setTos(tos);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::setMulticastOutputInterface(int interfaceId)
{
    cMessage *msg = new cMessage("SetMulticastOutputIf", UDP_C_SETOPTION);
    UDPSetMulticastInterfaceCommand *ctrl = new UDPSetMulticastInterfaceCommand();
    ctrl->setSockId(sockId);
    ctrl->setInterfaceId(interfaceId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::setMulticastLoop(bool value)
{
    cMessage *msg = new cMessage("SetMulticastLoop", UDP_C_SETOPTION);
    UDPSetMulticastLoopCommand *ctrl = new UDPSetMulticastLoopCommand();
    ctrl->setSockId(sockId);
    ctrl->setLoop(value);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::setReuseAddress(bool value)
{
    cMessage *msg = new cMessage("SetReuseAddress", UDP_C_SETOPTION);
    UDPSetReuseAddressCommand *ctrl = new UDPSetReuseAddressCommand();
    ctrl->setSockId(sockId);
    ctrl->setReuseAddress(value);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::joinMulticastGroup(const IPvXAddress& multicastAddr, int interfaceId)
{
    cMessage *msg = new cMessage("JoinMulticastGroups", UDP_C_SETOPTION);
    UDPJoinMulticastGroupsCommand *ctrl = new UDPJoinMulticastGroupsCommand();
    ctrl->setSockId(sockId);
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    ctrl->setInterfaceIdArraySize(1);
    ctrl->setInterfaceId(0, interfaceId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::joinLocalMulticastGroups()
{
    IInterfaceTable *ift = InterfaceTableAccess().get();
    unsigned int numOfAddresses = 0;
    for (int i = 0; i < ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data())
            numOfAddresses += ie->ipv4Data()->getJoinedMulticastGroups().size();
    }

    if (numOfAddresses > 0)
    {
        UDPJoinMulticastGroupsCommand *ctrl = new UDPJoinMulticastGroupsCommand();
        ctrl->setSockId(sockId);
        ctrl->setMulticastAddrArraySize(numOfAddresses);
        ctrl->setInterfaceIdArraySize(numOfAddresses);

        unsigned int k = 0;
        for (int i=0; i<ift->getNumInterfaces(); ++i)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            int interfaceId = ie->getInterfaceId();
            if (ie->ipv4Data())
            {
                const IPv4InterfaceData::IPv4AddressVector &addresses = ie->ipv4Data()->getJoinedMulticastGroups();
                for (unsigned int j = 0; j < addresses.size(); ++j, ++k)
                {
                    ctrl->setMulticastAddr(k, addresses[j]);
                    ctrl->setInterfaceId(k, interfaceId);
                }
            }
        }

        cMessage *msg = new cMessage("JoinMulticastGroups", UDP_C_SETOPTION);
        msg->setControlInfo(ctrl);
        sendToUDP(msg);
    }
}


void IUDPSocket::leaveMulticastGroup(const IPvXAddress& multicastAddr)
{
    cMessage *msg = new cMessage("LeaveMulticastGroups", UDP_C_SETOPTION);
    UDPLeaveMulticastGroupsCommand *ctrl = new UDPLeaveMulticastGroupsCommand();
    ctrl->setSockId(sockId);
    ctrl->setMulticastAddrArraySize(1);
    ctrl->setMulticastAddr(0, multicastAddr);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void IUDPSocket::leaveLocalMulticastGroups()
{
    IInterfaceTable *ift = InterfaceTableAccess().get();
    unsigned int numOfAddresses = 0;
    for (int i = 0; i < ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->ipv4Data())
            numOfAddresses += ie->ipv4Data()->getJoinedMulticastGroups().size();
    }

    if (numOfAddresses > 0)
    {
        UDPLeaveMulticastGroupsCommand *ctrl = new UDPLeaveMulticastGroupsCommand();
        ctrl->setSockId(sockId);
        ctrl->setMulticastAddrArraySize(numOfAddresses);

        unsigned int k = 0;
        for (int i=0; i<ift->getNumInterfaces(); ++i)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->ipv4Data())
            {
                const IPv4InterfaceData::IPv4AddressVector &addresses = ie->ipv4Data()->getJoinedMulticastGroups();
                for (unsigned int j = 0; j < addresses.size(); ++j, ++k)
                {
                    ctrl->setMulticastAddr(k, addresses[j]);
                }
            }
        }

        cMessage *msg = new cMessage("LeaveMulticastGroups", UDP_C_SETOPTION);
        msg->setControlInfo(ctrl);
        sendToUDP(msg);
    }
}


bool IUDPSocket::belongsToSocket(cMessage *msg)
{
    return dynamic_cast<UDPControlInfo *>(msg->getControlInfo()) &&
           ((UDPControlInfo *)(msg->getControlInfo()))->getSockId()==sockId;
}

bool IUDPSocket::belongsToAnyUDPSocket(cMessage *msg)
{
    return dynamic_cast<UDPControlInfo *>(msg->getControlInfo());
}

std::string IUDPSocket::getReceivedPacketInfo(cPacket *pk)
{
    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->getControlInfo());

    IPvXAddress srcAddr = ctrl->getSrcAddr();
    IPvXAddress destAddr = ctrl->getDestAddr();
    int srcPort = ctrl->getSrcPort();
    int destPort = ctrl->getDestPort();
    int interfaceID = ctrl->getInterfaceId();
    int ttl = ctrl->getTtl();
    int tos = ctrl->getTypeOfService();

    std::stringstream os;
    os << pk << " (" << pk->getByteLength() << " bytes) ";
    os << srcAddr << ":" << srcPort << " --> " << destAddr << ":" << destPort;
    os << " TTL=" << ttl << " ToS=" << tos << " on ifID=" << interfaceID;
    return os.str();
}

