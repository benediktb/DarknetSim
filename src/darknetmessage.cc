/*
 * darknetmessage.cc
 */

#include "darknetmessage.h"
#include <sstream>

Register_Class(DarknetMessage)
;

void DarknetMessage::copy(const DarknetMessage& other) {
    // Nothing to add here
}

std::string DarknetMessage::typeToString(DarknetMessageType t) {
    switch (t) {
    case DM_UNKNOWN:
        return std::string("DM_UNKNOWN");
    case DM_OTHER:
        return std::string("DM_OTHER");
    case DM_REQUEST:
        return std::string("DM_REQUEST");
    case DM_RESPONSE:
        return std::string("DM_RESPONSE");
    case DM_CON_SYN:
        return std::string("DM_CON_SYN");
    case DM_CON_ACK:
        return std::string("DM_CON_ACK");
    case DM_RCVACK:
        return std::string("DM_RCVACK");
    case DM_FORWARD:
        return std::string("DM_FORWARD");
    default:
        return std::string("!! UNKNOWN MSG TYPE");
    }
}

std::string DarknetMessage::toString(bool contentOnly) const {
    std::ostringstream out;
    if (!contentOnly)
        out << "[DarknetMessage, ";
    out << "type " << typeToString(type_var);
    out << ", requestMessageID " << requestMessageID_var;
    out << ", TTL " << TTL_var;
    if (!contentOnly)
        out << "]";

    return out.str();
}
