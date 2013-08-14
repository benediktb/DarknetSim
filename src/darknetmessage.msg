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


enum DarknetMessageType {
    DM_UNKNOWN = 0;
    
    //DATA MESSAGES
    DM_REQUEST = 10;
    DM_RESPONSE = 11;
    
    //MAINTENANCE_MESSAGES
    DM_CON_SYN = 20;
    DM_CON_ACK = 21;
    
    DM_RCVACK = 25;
    
    DM_FORWARD = 30;
}

packet DarknetMessage {
//    @customize(true);  // see the generated C++ header for more info
    string destNodeID;
    string srcNodeID;
    int TTL = 32;
    int type @enum(DarknetMessageType);
    string visitedNodes[];
    long requestMessageID; //if this is a Response: the ID of the Message which requested this
}
