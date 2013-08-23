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

#ifndef HOTPOTATONODE_H_
#define HOTPOTATONODE_H_

#include "DarknetOfflineDetectionNode.h"

class HotpotatoNode: public DarknetOfflineDetectionNode {
public:
    HotpotatoNode() : DarknetOfflineDetectionNode::DarknetOfflineDetectionNode() {};
   virtual ~HotpotatoNode() {};
protected:
   int requestFanout;
   std::set<long> answeredRequests;

   virtual std::vector<DarknetPeer*> findNextHop(DarknetMessage* msg);
   virtual void handleSelfMessage(cMessage *msg);
   virtual void initialize(int stage);
   virtual void handleRequest(DarknetMessage* request);

};

#endif /* HOTPOTATONODE_H_ */
