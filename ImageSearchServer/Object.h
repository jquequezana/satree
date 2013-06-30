#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Page.h"

class Object {
public:
    Object()
    : m_pBuffer(NULL)
    {}
    virtual ~Object()           { destroy(); }
    
    virtual Object*     copy() = 0;
    virtual size_t      gethashCode() const = 0;
    virtual short       equal(Object* obj) const = 0;
    virtual void        unSerialize(const byte *data, size_t n) = 0;
    virtual const byte* serialize() const = 0;
    virtual size_t      serializeSize() const = 0;
protected:
    void                destroy()   {   delete m_pBuffer; m_pBuffer = NULL;}
    mutable byte*       m_pBuffer;
};

#endif //