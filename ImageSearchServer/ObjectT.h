#ifndef OBJECT_TEMPLATE_H_
#define OBJECT_TEMPLATE_H_


#include <iostream>
#include "Object.h"

template<typename ValueTy>
class ObjectT : public Object
{
public:
    ObjectT(const ValueTy& info = ValueTy() )
    : Object(), m_Data(info)
    {}
    ObjectT(const ObjectT& info )
    : Object(), m_Data(info.m_Data)
    {}
    virtual ~ObjectT(){}
    virtual Object*     copy()                      {return new ObjectT(); }
    virtual size_t      gethashCode() const         {return 0;}
    virtual short       equal(Object* obj) const    {return m_Data == ((ObjectT*)obj)->m_Data; }
    virtual void        unSerialize(const byte *data, size_t n);
    virtual const byte* serialize() const ;
    virtual size_t      serializeSize() const   { return sizeof(m_Data);        }
    
    bool operator == (const ObjectT& it) const  { return m_Data == it.m_Data;   }
    bool operator != (const ObjectT& it) const  { return !(m_Data == it.m_Data);}
    
    bool operator < (const ObjectT& it) const    { return m_Data < it.m_Data; }
        bool operator >= (const ObjectT& it) const   { return !(m_Data < it.m_Data);  }
        
        bool operator <= (const ObjectT& it) const   { return m_Data <= it.m_Data;}
            bool operator > (const ObjectT& it) const    { return !(m_Data <= it.m_Data) ;}
            
            friend std::ostream& operator << (std::ostream& out, const ObjectT& obj)
            {
                out << obj.m_Data ;
                return out;
            }
            
        public:
            ValueTy m_Data;
        };
        
        template<typename ValueTy>
        void ObjectT<ValueTy>::unSerialize(const byte *data, size_t n)
        {
            ValueTy *p = (ValueTy *)data;
            m_Data = p[0];
            if ( !m_pBuffer )
                delete [] m_pBuffer;
        }
        
        template<typename ValueTy>
        const byte* ObjectT<ValueTy>::serialize() const
        {
            if( !m_pBuffer )
            {
                m_pBuffer = new byte[ serializeSize () ];
                
                memcpy(m_pBuffer, &m_Data, serializeSize () ); 
                //m_Data.serialize(m_pBuffer); 
            } 
            return m_pBuffer; 
        }    
        
#endif //OBJECT_TEMPLATE_H_