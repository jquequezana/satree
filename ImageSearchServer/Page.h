#ifndef _PAGE_H_
#define _PAGE_H_

#pragma once

#include <cstdlib>
#include <cstring>

//namespace slim_tree
//{
#include "defines.h"
/**
 * maneja mejor el byte*
 */
class Page{
public:
    Page(){}
    Page(const Page &p)
    :   m_PageID(p.m_PageID),
    m_BufferSize(p.m_BufferSize)
    {
        m_Buffer = new byte[m_BufferSize];
        memcpy(m_Buffer, p.m_Buffer, m_BufferSize );
    }
    Page (size_t size, long pageid = -1);
    virtual ~Page();
    void            copy (Page * page);
    virtual void    write (const byte* buff, size_t n, size_t offset = 0);
    void            clear ();
    
    void    setPageID(PageID pageID) { m_PageID = pageID;   }
    void    setPageSize(ulong size) { m_BufferSize = size;  }
    byte*   getData() const         { return m_Buffer;      }
    ulong   getPageID() const       { return m_PageID;      }
    size_t  getPageSize() const     { return m_BufferSize;  }
private:
    byte*           m_Buffer;
    size_t          m_BufferSize;
    long            m_PageID;
};

Page::Page(size_t size, long pageid /* = 0 */)
: m_BufferSize(size),
m_PageID(pageid)
{
    m_Buffer = new byte[size];
}
void Page::write(const byte* buff, size_t n, size_t offset)
{
    memcpy((void *)(m_Buffer + offset), (void*)buff, n); // m_Buffer[i]<<0;
}
void Page::copy(Page * page)
{
    memcpy(this->getData(), page->getData(), this->getPageSize());
}
void Page::clear()
{
    memset(this->getData(), 0, this->getPageSize());
}
Page::~Page()
{
    if(m_Buffer != NULL )
        delete [] m_Buffer;
    m_Buffer = NULL;
}
//}

#endif //_PAGE_H_