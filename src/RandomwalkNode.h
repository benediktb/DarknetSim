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

#ifndef RANDOMWALKODE_H_
#define RANDOMWALKNODE_H_

#include "DarknetOfflineDetectionNode.h"
#include "DarknetSimpleNode.h"

//class RandomwalkNode: public DarknetOfflineDetectionNode {
class RandomwalkNode: public DarknetSimpleNode {
public:
//    RandomwalkNode() : DarknetOfflineDetectionNode::DarknetOfflineDetectionNode() {};
    RandomwalkNode() : DarknetSimpleNode::DarknetSimpleNode() {};
   virtual ~RandomwalkNode() {};
protected:
   int requestFanout;
   std::set<long> answeredRequests;

   virtual std::vector<DarknetPeer*> findNextHop(DarknetMessage* msg);
   virtual void handleSelfMessage(cMessage *msg);
   virtual void initialize(int stage);
   virtual void handleRequest(DarknetMessage* request);

};

#endif /* RANDOMWALKNODE_H_ */
