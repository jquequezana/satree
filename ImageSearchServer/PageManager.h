#ifndef _PAGE_MANAGER_H_
#define  _PAGE_MANAGER_H_

#pragma once

#include <cstdio>
#include <map>
#include <vector>
#include <stack>
#include <cassert>
#include <fstream>

#include "defines.h"
#include "Page.h"


//namespace PageManager
//{
class DiskPageManager;

//[ StorageInfo | HeaderInfo | Data0, Data1 Data2 Data3 ]
class Storage{
    friend class DiskPageManager;
    
public:
    Storage();
    ~Storage()
    {
        close();
    }
    
    bool  open(const char* fName);
    void  close();
    void  create(const char* fName, size_t pageSize, size_t headerSize = 0);
    void  readPage(const ulong &pageId, void *page);
    void  writePage(const ulong &pageId, void *page);
    void  writeHeader(void *page);
    void  readHeader(void *page);
    ulong insertNewPage(void *page);
    void  flushCache ();
    void  freePage(const ulong& pageId);
    bool  isEmpty() const       { return m_fHandle == NULL; }
    size_t getPageCount() const { return m_Info.pageCount;  }
    void    updateFirstDiskAddress(size_t headerSize);
    
protected:
    long toFileCursor(const ulong& pageId);
    ulong toPageID(const ulong& fileCursor);
private:
    struct StorageInfo{
        size_t      pageCount;
        size_t      pageSize;
        size_t      readCount;
        size_t      writeCount;
        size_t      headerSize;
    };
private:
    FILE*       m_fHandle;
    long        m_firstPageDiskAddress;
    StorageInfo m_Info;
};

Storage::Storage()
{
    m_fHandle = NULL;
    m_Info.pageSize = 0;
    m_Info.readCount = 0;
    m_Info.writeCount = 0;
    m_Info.headerSize = 0;
    m_Info.pageCount = 0;
    m_firstPageDiskAddress = 0;
}
void Storage::updateFirstDiskAddress(size_t headerSize)
{
    m_firstPageDiskAddress = sizeof(StorageInfo) + headerSize;
    m_Info.headerSize = headerSize;
}
bool Storage::open(const char* fName)
{
    m_fHandle = fopen(fName,"r+b");
    if(m_fHandle == NULL )
        return false;
    fseek(m_fHandle, 0, SEEK_SET);
    int ret = fread(&m_Info, sizeof(StorageInfo), 1, m_fHandle);
    assert( ret == 1);
    
    m_Info.pageSize = 256;
    m_Info.headerSize = 0;
    
    return true;
}
void Storage::close()
{
    fseek(m_fHandle, 0, SEEK_SET);
    int ret = fwrite(&m_Info, sizeof(StorageInfo), 1, m_fHandle);
    assert( ret == 1);
    
    ret = fflush(m_fHandle);
    assert(ret != EOF );
    fclose(m_fHandle);
}
void Storage::create(const char* fName, size_t pageSize, size_t headerSize /*= 0 */)
{
    m_Info.pageSize = pageSize;
    m_Info.headerSize = headerSize;
    m_firstPageDiskAddress = sizeof(StorageInfo) + headerSize;
    
    m_fHandle = fopen(fName, "w+b");
    assert(m_fHandle != NULL );
}
void Storage::readPage(const ulong& pageId, void* page)
{
    fseek(m_fHandle, toFileCursor(pageId), SEEK_SET);
    int ret = fread(page, m_Info.pageSize ,1 ,m_fHandle);
    assert( ret == 1);
    m_Info.readCount++;
}
void Storage::writePage(const ulong &pageId, void *page)
{
    fseek(m_fHandle, toFileCursor(pageId), SEEK_SET);
    int ret = fwrite(page, m_Info.pageSize, 1, m_fHandle);
    assert( ret == 1);
    m_Info.writeCount++;
}
void Storage::writeHeader(void *page)
{
    assert(m_Info.headerSize > 0);
    fseek(m_fHandle, sizeof(StorageInfo), SEEK_SET);
    int ret = fwrite(page, m_Info.headerSize, 1, m_fHandle);
    assert( ret == 1);
}
void Storage::readHeader(void *page)
{
    assert(m_Info.headerSize > 0);
    fseek(m_fHandle, sizeof(StorageInfo), SEEK_SET);
    int ret = fread(page, m_Info.headerSize, 1, m_fHandle);
    assert( ret == 1);
}
long Storage::toFileCursor(const ulong& pageId)
{
    return (m_firstPageDiskAddress + (pageId * m_Info.pageSize));
}

ulong Storage::toPageID(const ulong& fileCursor)
{
    return (fileCursor - m_firstPageDiskAddress)/m_Info.pageSize;
}

ulong Storage::insertNewPage(void *page)
{
    fseek(m_fHandle, m_firstPageDiskAddress, SEEK_END);
    ulong pos = ftell(m_fHandle);
    int ret = fwrite(page, m_Info.pageSize, 1, m_fHandle);
    assert( ret == 1 );
    m_Info.pageCount++;
    long id = toPageID(pos);
    return id;//toPageID(pos);
}
void  Storage::flushCache ()
{
    int ret = fflush(m_fHandle);
    assert(ret != EOF );
}

void  Storage::freePage(const ulong& pageId)
{
    //fseek(m_fHandle, toFileCursor(pageId), SEEK_SET );
    //fwrite()
}


/**
 * class PageManager
 * @version 1.0
 * @author Alexander Ocsa.
 **/
class PageManager{
public:
    enum DBAccessMode {
        DB_READONLY,          //!< Open file with read access only
        DB_WRITEONLY,         //!< Open file with write access only
        DB_READWRITE,         //!< Open file with read and write access
        DB_CREATE,            //!< Create the file if it does not exist
        DB_NO_CREATE,         //!< Do not create the file if it does not exist
        DB_TRUNCATE,          //!< Truncate the file
        DB_APPEND,            //!< Append to the file
    };
    
public:
    //LIFECYCLE
    PageManager()   {m_firstPos = 0;}
    PageManager(const PageManager& obj);
    
    virtual ~PageManager(){}
    
    //OPERATORS
    PageManager& operator = (const PageManager& obj);
    
    // OPERATIONS
    //virtual void  ReleasePage (Page *page)=0;
    virtual void    writePage   (Page *page)=0;
    virtual void    disposePage (Page *page)=0;
    
    // ACCESS
    
    // INQUIRY
    
    virtual Page* getPage (PageID pageid)=0;
    virtual Page* getHeaderPage() = 0;
    virtual void  writeHeaderPage(Page* hPage) = 0;
    virtual Page* getNewPage ()=0;
    virtual bool    isEmpty () const =0;
    virtual void    setHeaderSize(long first) = 0;
    DBAccessMode    getAccessMode() { return mode;  }
    virtual size_t  getBufferSize() const = 0;
    
protected:
    size_t          m_firstPos;
    DBAccessMode    mode;
};

// [info] [header] [ data ]
class DiskPageManager: public PageManager{
public:
    DiskPageManager()
    : PageManager(),
    m_Storage()
    {
        //m_storage = new Storage();
    }
    ~DiskPageManager()
    {
    }
    
    // OPERATIONS
    bool open(const char* fName)    { mode = DB_READWRITE; return m_Storage.open(fName);    }
    void close()                    { m_Storage.close();        }
    void create(const char* fName, size_t pageSize, size_t headerSize = 0)  { mode = DB_CREATE ; m_Storage.create(fName, pageSize, headerSize); }
    
    virtual void    writePage   (Page *page);
    virtual Page*   getPage (PageID pageid);
    virtual Page*   getNewPage ();
    virtual void    disposePage (Page *page)    {}
    virtual bool    isEmpty() const     {return m_Storage.m_fHandle == NULL;    }
    virtual Page*   getHeaderPage();
    virtual void    writeHeaderPage(Page* hPage);
    
    virtual void   setHeaderSize(long headerSize)
    {
        m_Storage.updateFirstDiskAddress(headerSize);
    }
    virtual size_t  getBufferSize() const { return m_Storage.m_Info.pageSize; }
    
private:
    Storage     m_Storage;
};

void DiskPageManager::writePage   (Page *page)
{
    assert(page->getPageSize() <= m_Storage.m_Info.pageSize );
    m_Storage.writePage(page->getPageID(), page->getData());
}
Page* DiskPageManager::getPage (PageID pageid)
{
    Page* page = new Page(m_Storage.m_Info.pageSize, pageid);
    m_Storage.readPage(pageid, page->getData());
    return page;
}
void DiskPageManager::writeHeaderPage(Page* hPage)
{
    //std::cout << hPage->getPageSize() <<" " <<  m_Storage.m_Info.headerSize <<std::endl;
    assert(hPage->getPageSize() <= m_Storage.m_Info.headerSize );
    m_Storage.writeHeader(hPage->getData());
}
Page* DiskPageManager::getHeaderPage()
{
    assert(m_Storage.m_Info.headerSize > 0);
    Page * page = new Page(m_Storage.m_Info.headerSize);
    m_Storage.readHeader(page->getData());
    return page;
}
Page* DiskPageManager::getNewPage ()
{
    Page* page = new Page(m_Storage.m_Info.pageSize);
    ulong id = m_Storage.insertNewPage(page->getData());
    page->setPageID(id);
    //std::cout <<"new page: " <<  id <<std::endl;
    
    return page;
}

class MemoryPageManager : public PageManager{
public:
    MemoryPageManager() : PageManager() {
        mode = PageManager::DB_CREATE;
        m_headerPage = NULL;
        m_sizePage = 1024;
    }
    //~MemoryPageManager(){}
    
    virtual void    writePage   (Page *page);
    virtual Page*   getPage (PageID pageid);
    virtual Page*   getNewPage ();
    virtual bool    isEmpty() const { return m_pages.empty();   }
    
    virtual Page*   getHeaderPage();                //{ return NULL;}
    virtual void    writeHeaderPage(Page* hPage);   //{  }
    
    virtual size_t headerSize() const   { return 0;};
    virtual void   setHeaderSize(long first) {}
    virtual void   disposePage(Page *page) {}
protected:
    
private:
    std::vector<Page *>   m_pages;
    Page*               m_headerPage;
    size_t              m_sizePage;
    std::stack<size_t>    m_freePages;
};

void MemoryPageManager::writePage   (Page *page)
{
    assert(page->getPageID() < m_pages.size() );
    m_pages[page->getPageID()] = new Page(*page);
}
Page* MemoryPageManager::getPage (PageID pageid)
{
    assert(pageid < m_pages.size() );
    return m_pages[pageid];
}

Page* MemoryPageManager::getNewPage ()
{
    Page* page = new Page(m_sizePage);
    page->setPageID( m_pages.size() );
    m_pages.push_back(page);
    return page;
}
Page* MemoryPageManager::getHeaderPage()
{
    return m_headerPage;
}
void MemoryPageManager::writeHeaderPage(Page* hPage)
{
    if( !m_headerPage)
        m_headerPage = new Page(hPage->getPageSize());
    memcpy(m_headerPage->getData(), hPage->getData(), hPage->getPageSize() );
}

//}

#endif //_PAGE_MANAGER_H_