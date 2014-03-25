/*
 * darknetmessage.h
 *
 */

#ifndef DARKNETMESSAGE_H_
#define DARKNETMESSAGE_H_

#include "darknetmessage_m.h"

class DarknetMessage: public DarknetMessage_Base {
private:
    void copy(const DarknetMessage& other);

public:
    DarknetMessage(const char *name = NULL, int kind = 0) :
            DarknetMessage_Base(name, kind) {
    }
    DarknetMessage(const DarknetMessage& other) :
            DarknetMessage_Base(other) {
        copy(other);
    }
    DarknetMessage& operator=(const DarknetMessage& other) {
        if (this == &other)
            return *this;
        DarknetMessage_Base::operator=(other);
        copy(other);
        return *this;
    }
    virtual DarknetMessage *dup() const {
        return new DarknetMessage(*this);
    }
    // ADD CODE HERE to redefine and implement pure virtual functions from DarknetMessage_Base

    static std::string typeToString(const DarknetMessageType t);
    virtual std::string toString(bool contentOnly = false) const;
};

#endif /* DARKNETMESSAGE_H_ */
