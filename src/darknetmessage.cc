/*
 * darknetmessage.cc
 */

#include "darknetmessage.h"
#include <sstream>

void DarknetMessage::copy(const DarknetMessage& other) {
    this->destNodeID_var = other.destNodeID_var;
    this->srcNodeID_var = other.srcNodeID_var;
    this->TTL_var = other.TTL_var;
    this->type_var = other.type_var;
    this->requestMessageID_var = other.requestMessageID_var;
}


std::string* DarknetMessage::typeToString(DarknetMessageType t) {
    switch (t) {
        case DM_UNKNOWN:
            return new std::string("DM_UNKNOWN");
        case DM_REQUEST:
            return new std::string("DM_REQUEST");
        case DM_RESPONSE:
            return new std::string("DM_RESPONSE");
        case DM_CON_SYN:
            return new std::string("DM_CON_SYN");
        case DM_CON_ACK:
            return new std::string("DM_CON_ACK");
        case DM_RCVACK:
            return new std::string("DM_RCVACK");
        case DM_FORWARD:
            return new std::string("DM_FORWARD");
        default:
            return new std::string("!! UNKNOWN MSG TYPE");
    }
}

std::string DarknetMessage::toString(bool contentOnly) const {
    std::ostringstream out;
    if (!contentOnly)
        out << "[DarknetMessage, ";
    out << "type " << *typeToString(type_var);
    out << ", requestMessageID " << requestMessageID_var;
    out << ", TTL " << TTL_var;
    if (!contentOnly)
        out << "]";

    return out.str();
}
