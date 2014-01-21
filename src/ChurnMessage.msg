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

cplusplus {{

#include <cmodule.h>

// DarknetChurnNode is meant by cModule, but due to circular dependencies, it
//   can't be defined like that.
typedef cModule *NodePtr;

enum ChurnMessageType {
    CHURN_GO_ON = 0,
    CHURN_GO_OFF = 1
};

}};

class noncobject NodePtr;
class noncobject ChurnMessageType;

message ChurnMessage {
    NodePtr node;
    ChurnMessageType type;
}
