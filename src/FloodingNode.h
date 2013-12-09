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

#ifndef FLOODINGNODE_H_
#define FLOODINGNODE_H_

#include "DarknetSimpleNode.h"

class FloodingNode: public DarknetSimpleNode {
public:
    FloodingNode() : DarknetSimpleNode::DarknetSimpleNode() {};
    virtual ~FloodingNode() {};
protected:
    std::set<long> seenMessages;
    simsignal_t sigDropAlreadySeen;

    virtual void initialize(int stage);
    virtual bool sendMessage(DarknetMessage* msg);
    virtual void handleDarknetMessage(DarknetMessage* msg, DarknetPeer *sender);
    virtual std::vector<DarknetPeer*> findNextHop(DarknetMessage* msg);
};

#endif /* FLOODINGNODE_H_ */
