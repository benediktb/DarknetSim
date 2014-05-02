#ifndef MESSAGECALLBACK_H_
#define MESSAGECALLBACK_H_

class MessageCallback {
public:
    virtual ~MessageCallback() {
    }

    virtual void callForMessage(cMessage* msg) = 0;
};

#endif /* MESSAGECALLBACK_H_ */
