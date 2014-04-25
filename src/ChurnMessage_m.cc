//
// Generated file, do not edit! Created by opp_msgc 4.3 from ChurnMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "ChurnMessage_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




Register_Class(ChurnMessage);

ChurnMessage::ChurnMessage(const char *name, int kind) : cMessage(name,kind)
{
}

ChurnMessage::ChurnMessage(const ChurnMessage& other) : cMessage(other)
{
    copy(other);
}

ChurnMessage::~ChurnMessage()
{
}

ChurnMessage& ChurnMessage::operator=(const ChurnMessage& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void ChurnMessage::copy(const ChurnMessage& other)
{
    this->node_var = other.node_var;
    this->type_var = other.type_var;
}

void ChurnMessage::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->node_var);
    doPacking(b,this->type_var);
}

void ChurnMessage::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->node_var);
    doUnpacking(b,this->type_var);
}

NodePtr& ChurnMessage::getNode()
{
    return node_var;
}

void ChurnMessage::setNode(const NodePtr& node)
{
    this->node_var = node;
}

ChurnMessageType& ChurnMessage::getType()
{
    return type_var;
}

void ChurnMessage::setType(const ChurnMessageType& type)
{
    this->type_var = type;
}

class ChurnMessageDescriptor : public cClassDescriptor
{
  public:
    ChurnMessageDescriptor();
    virtual ~ChurnMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(ChurnMessageDescriptor);

ChurnMessageDescriptor::ChurnMessageDescriptor() : cClassDescriptor("ChurnMessage", "cMessage")
{
}

ChurnMessageDescriptor::~ChurnMessageDescriptor()
{
}

bool ChurnMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<ChurnMessage *>(obj)!=NULL;
}

const char *ChurnMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int ChurnMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int ChurnMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *ChurnMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "node",
        "type",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int ChurnMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='n' && strcmp(fieldName, "node")==0) return base+0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *ChurnMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "NodePtr",
        "ChurnMessageType",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *ChurnMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int ChurnMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    ChurnMessage *pp = (ChurnMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string ChurnMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    ChurnMessage *pp = (ChurnMessage *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getNode(); return out.str();}
        case 1: {std::stringstream out; out << pp->getType(); return out.str();}
        default: return "";
    }
}

bool ChurnMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    ChurnMessage *pp = (ChurnMessage *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ChurnMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        "NodePtr",
        "ChurnMessageType",
    };
    return (field>=0 && field<2) ? fieldStructNames[field] : NULL;
}

void *ChurnMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    ChurnMessage *pp = (ChurnMessage *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getNode()); break;
        case 1: return (void *)(&pp->getType()); break;
        default: return NULL;
    }
}


