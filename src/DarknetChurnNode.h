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

class DarknetChurnNode: public DarknetOfflineDetectionNode {

protected:
    ChurnController* churnController;
    bool goOnline;

    /** Distribution for online time (time till next offline time) */
    IRandomDistribution* onTimeDistribution;

    /** Distribution for offline time (time till next online time) */
    IRandomDistribution* offTimeDistribution;

    // To extend/overwrite:
    virtual void initialize(int stage);

    virtual bool startApp(IDoneCallback *doneCallback);
    virtual bool stopApp(IDoneCallback *doneCallback);
    virtual bool crashApp(IDoneCallback *doneCallback);

    virtual std::vector<DarknetPeer*> findNextHop(DarknetMessage* msg);

public:

    DarknetChurnNode() : DarknetOfflineDetectionNode::DarknetOfflineDetectionNode(), goOnline(false) {};
    virtual ~DarknetChurnNode() {};

    // To be used by ChurnController
    void setGoOnline(bool goOnline);
    bool getStartState() const;
    IRandomDistribution* getOnTimeDistribution() const;
    IRandomDistribution* getOffTimeDistribution() const;

};

#endif /* DARKNETCHURNNODE_H_ */
