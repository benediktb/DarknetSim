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

#ifndef DARKNETSIMPLENODE_H_
#define DARKNETSIMPLENODE_H_

#include "DarknetBaseNode.h"

class PingTimer: public cMessage {
public:
    PingTimer(std::string name) : cMessage(name.c_str()) {};
};

class DarknetSimpleNode: public DarknetBaseNode {
public:
    DarknetSimpleNode() : DarknetBaseNode::DarknetBaseNode() {};
    virtual ~DarknetSimpleNode() {};
protected:
    virtual int numInitStages() const { return 6; }
    virtual void initialize(int stage);
    virtual void handleSelfMessage(cMessage* msg);
    virtual void handleIncomingMessage(DarknetMessage* msg);
    virtual void connectPeer(std::string nodeID);
};

#endif /* DARKNETSIMPLENODE_H_ */
