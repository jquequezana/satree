#include <iostream>

#include "defines.h"
#include "PageManager.h"

namespace mds
{
    template <typename Conteiner ,typename KeyType>
    int binarySearch(Conteiner Array ,int size ,const KeyType &key);
    /**
     * author: Alexander Ocsa
     */
    template<typename ObjectType>
    class BTree
    {
    public:
        class iterator;
        struct Entry
        {
            Entry(ObjectType* ptr, const PageID& id)
            : m_pObj(ptr), m_Ptr(id){}
            ObjectType* m_pObj;
            PageID      m_Ptr;  //  || ObjID
            ObjectType& operator * () { return *m_pObj; }
        };
        
    public:
        typedef PageID  id_type;
        typedef Entry   value_type;
        
    private:
        struct TreeInfo;
        enum b_state{BT_OVERFLOW, BT_OK, BT_DUPLICATE};
        
        class BPage{
        public:
            struct PageInfo;
        public:
            BPage()
            {
                m_Info.m_Next = -1;
                m_Info.m_ItemCount = 0;
            }
            BPage(const BPage& node)
            {
                m_Page    = new Page( *node.m_Page );
                m_Info    = node.m_Info;
                order     = node.order;
                
                m_Entries = (Entry *)(m_Page->getData () + sizeof(PageInfo));
                
                for (size_t i=0; i<m_Info.m_ItemCount; i++ )
                    m_Entries[i].m_pObj= getObject(i);
                
            }
            BPage(Page* page, const size_t &n)
            {
                m_Page = page;
                m_Entries = (Entry *)(m_Page->getData () + sizeof(PageInfo));
                m_Info.m_ItemCount = 0;
                m_Info.m_Next = -1;
                order = n;
            }
            ~BPage()
            {
                clear();
                delete m_Page;
            }
            void clear()        // limpia la informacion
            {
                m_Info.m_ItemCount = 0;
                for (size_t i=0; i<m_Info.m_ItemCount; i++ )
                    delete m_Entries[i].m_pObj;
            }
            void setObject(ObjectType* obj, size_t pos)
            {
                size_t sizeObj = obj->serializeSize();
                size_t firstPos = sizeof(PageInfo) + (order+1)*sizeofEntry();
                memcpy ( m_Page->getData () + firstPos + sizeObj*pos, obj->serialize(), sizeObj );
            }
            void insertSubTree(ObjectType* obj, size_t pos)
            {
                assert ( pos <= order );
                if ( pos != m_Info.m_ItemCount )
                {
                    const size_t objectSize = obj->serializeSize();
                    const size_t firstPos   = sizeof(PageInfo) +  (order+1)*sizeofEntry();
                    const size_t tam        = (m_Info.m_ItemCount - pos)*objectSize;
                    byte* source = new byte[tam];
                    memcpy( source, m_Page->getData() + firstPos + pos*objectSize, tam);
                    byte* target = (byte* )(m_Page->getData() + firstPos + (pos+1)*objectSize );
                    memcpy(target, source, tam);
                }
                size_t i;
                for(i = count () ; i > pos; i--){
                    getChild(i+1) = m_Entries[i].m_Ptr;
                    m_Entries[i].m_pObj = new ObjectType(*m_Entries[i-1].m_pObj);
                }
                
                getChild(i+1) = m_Entries[i].m_Ptr;
                m_Entries[ pos ].m_pObj = new ObjectType(*obj);
                size_t sizeObj = obj->serializeSize();
                size_t firstPos = sizeof(PageInfo) + (order+1)*sizeofEntry();
                memcpy ( m_Page->getData () + firstPos + sizeObj*pos, obj->serialize(), sizeObj );
                
                ++m_Info.m_ItemCount;
            }
            void insertInLeaf(const value_type& info, size_t& pos)
            {
                assert ( pos <= order );
                PageID next = m_Info.m_Next;
                if ( pos != m_Info.m_ItemCount )
                {
                    const size_t objectSize = info.m_pObj->serializeSize();
                    const size_t firstPos   = sizeof(PageInfo) +  (order+1)*sizeofEntry();
                    const size_t tam        = (m_Info.m_ItemCount - pos)*objectSize;
                    byte* source = new byte[tam];
                    memcpy( source, m_Page->getData() + firstPos + pos*objectSize, tam);
                    byte* target = (byte* )(m_Page->getData() + firstPos + (pos+1)*objectSize );
                    memcpy(target, source, tam);
                }
                
                size_t i;
                for(i = count (); i > pos; i--){
                    getChild(i+1) = m_Entries[i].m_Ptr;
                    m_Entries[i].m_pObj  =  new ObjectType(*m_Entries[i-1].m_pObj);
                }
                getChild(i+1) = getChild(i);
                
                m_Entries[ pos ].m_pObj = new ObjectType(*info.m_pObj);
                
                size_t sizeObj = info.m_pObj->serializeSize();
                size_t firstPos = sizeof(PageInfo) + (order+1)*sizeofEntry();
                memcpy ( m_Page->getData () + firstPos + sizeObj*pos, info.m_pObj->serialize(), sizeObj );
                
                m_Info.m_Next = next;
                m_Entries[ pos ].m_Ptr = info.m_Ptr;
                ++m_Info.m_ItemCount;
            }
            void insert(const value_type& info, size_t& pos)
            {
                assert ( pos <= order );
                if ( pos != m_Info.m_ItemCount )
                {
                    const size_t objectSize = info.m_pObj->serializeSize();
                    const size_t firstPos   = sizeof(PageInfo) +  (order+1)*sizeofEntry();
                    const size_t tam        = (m_Info.m_ItemCount - pos)*objectSize;
                    byte* source = new byte[tam];
                    memcpy( source, m_Page->getData() + firstPos + pos*objectSize, tam);
                    byte* target = (byte* )(m_Page->getData() + firstPos + (pos+1)*objectSize );
                    memcpy(target, source, tam);
                }
                
                size_t i;
                for(i = count (); i > pos; i--){
                    getChild(i+1) = m_Entries[i].m_Ptr;
                    m_Entries[i].m_pObj  =  new ObjectType(*m_Entries[i-1].m_pObj);
                }
                getChild(i+1) = getChild(i);
                
                m_Entries[ pos ].m_pObj = new ObjectType(*info.m_pObj);
                
                size_t sizeObj = info.m_pObj->serializeSize();
                size_t firstPos = sizeof(PageInfo) + (order+1)*sizeofEntry();
                memcpy ( m_Page->getData () + firstPos + sizeObj*pos, info.m_pObj->serialize(), sizeObj );
                
                m_Entries[ pos ].m_Ptr = info.m_Ptr;
                ++m_Info.m_ItemCount;
            }
            void push_back(const value_type& info)
            {
                size_t sizeObj = info.m_pObj->serializeSize();
                size_t firstPos = sizeof(PageInfo) + (order+1)*sizeofEntry();
                memcpy ( m_Page->getData () + firstPos + sizeObj*m_Info.m_ItemCount, info.m_pObj->serialize(), sizeObj );
                
                m_Entries[ m_Info.m_ItemCount ].m_pObj = new ObjectType(*info.m_pObj);
                m_Entries[ m_Info.m_ItemCount++ ].m_Ptr = info.m_Ptr;
            }
            
            PageID  pageID() const  { return m_Page->getPageID ();   }
            Entry*  getEntries()    { return m_Entries;             }
            size_t  count() const   { return m_Info.m_ItemCount;    }
            void    reset()         { m_Info.m_ItemCount = 0;       }
            void    unSerialize(Page* pag, const size_t &n)
            {
                m_Page = pag;
                order = n;
                memcpy (&m_Info, pag->getData (), sizeof(PageInfo));
                m_Entries = (Entry *)(m_Page->getData () + sizeof(PageInfo));
                
                for (size_t i=0; i<m_Info.m_ItemCount; i++)
                    m_Entries[i].m_pObj= getObject(i);
            }
            PageID  getChild(size_t pos) const  { return pos > order?m_Info.m_Next : m_Entries[pos].m_Ptr;   }
            PageID& getChild(size_t pos)        { return pos > order?m_Info.m_Next : m_Entries[pos].m_Ptr;   }
            
            ObjectType* getObject(size_t pos)
            {
                assert ( pos <= order);
                ObjectType* obj = new ObjectType;
                size_t firstPos = sizeof(PageInfo) + (order + 1)*sizeofEntry();
                size_t objectSize = obj->serializeSize();
                obj->unSerialize( m_Page->getData() + firstPos + pos*objectSize, objectSize);
                return obj;
            }
            Page*   serialize()
            {
                memcpy (m_Page->getData (), &m_Info, sizeof(PageInfo));
                return m_Page;
            }
            size_t sizeofEntry() const { return sizeof(Entry); }
            void print()
            {
                std::cout << "ID: " << pageID() << "\n";
                size_t i;
                for ( i=0; i<count(); i++)
                {
                    std::cout << "(" << *getObject(i) << ", " << m_Entries[i].m_Ptr << ")";
                }
                std::cout << "(" << getChild(i) << ")";
                std::cout << "\n";
            }
        public:
            struct PageInfo{
                size_t  m_ItemCount;
                PageID  m_Next;
            };
            PageInfo    m_Info;
            Entry*      m_Entries;
            Page*       m_Page;
            size_t      order;
        };
        
    public:
        BTree() {}
        BTree(PageManager* pageManager, bool duplicate = false);
        ~BTree();
        void        setPageManager(PageManager* pm, bool duplicate = false);
        void        insert(const value_type& obj);
        id_type     find(const ObjectType& info);
        size_t&     order()  { return m_Info.m_Order;   }
        iterator    find(ObjectType& obj);
        iterator    findLess(ObjectType& obj);
        iterator    findMajor(ObjectType& obj);
        iterator    begin();
        iterator    end();
        void        print(std::ostream& out);
        
    private:
        b_state     insert(BPage& ptr, const value_type& obj, size_t level);
        void        splitRoot();
        void        split(BPage& parent, BPage& child, size_t& pos, size_t& level);
        void        initialize();
        Page*       getNewPage()
        {
            m_Info.m_Count++;
            return m_PageManager->getNewPage ();
        }
        bool        isOverflow(BPage& node) const       { return node.count () > m_Info.m_Order; }
        void        print(BPage& ptr, std::ostream& out, size_t level);
        
    public:
        class iterator{
            int         m_Pos;
            size_t      m_Order;
            BPage       m_Node;
            PageManager* m_PageManager;
            
        public:
            iterator(const BPage& p, size_t &order, int pos, PageManager* pm)
            :m_Node(p), m_Order(order), m_Pos(pos), m_PageManager(pm)
            {
            }
            iterator(const iterator& iter)
            :m_Node(iter.m_Node), m_Order(iter.m_Order),
            m_Pos(iter.m_Pos), m_PageManager(iter.m_PageManager)
            {
            }
            iterator operator++(int)
            {
                if( m_Node.m_Info.m_Next == -1)
                {
                    if ( m_Pos < m_Node.count ()-1 )
                        m_Pos++;
                    else{
                        m_Node.clear();
                        m_Node.m_Info.m_Next = 0;
                        m_Pos = -1;
                    }
                }
                else if (m_Pos < m_Node.count ()-1 )
                {
                    m_Pos++;
                }
                else
                {
                    PageID next = m_Node.m_Info.m_Next;
                    m_Node.clear();
                    m_Node.unSerialize( m_PageManager->getPage(next) , m_Order);
                    m_Pos = 0;
                    //std::cout << std::endl;
                }
                return *this;
            }
            void write()
            {
                m_Node.setObject( m_Node.m_Entries[m_Pos].m_pObj , m_Pos);
                m_PageManager->writePage(m_Node.serialize());
            }
            Entry& operator*()  { return m_Node.m_Entries[m_Pos];       }
            bool operator == (const iterator& iter) const
            {
                return (m_Node.pageID() == iter.m_Node.pageID() ) && (m_Pos == iter.m_Pos);
            }
            bool operator != (const iterator& iter) const //falta q solo uno de ellos q sea diferente
            {
                return (m_Pos != iter.m_Pos) || (m_Node.pageID() != iter.m_Node.pageID() );
            }
            
        };
        
    private:
        struct TreeInfo{
            size_t  m_Heigth;
            size_t  m_Order;
            size_t  m_Count;
            bool    m_bDuplicate;
        };
        
        BPage*          m_pRoot;
        PageManager*    m_PageManager;
        TreeInfo        m_Info;
        
        iterator*       m_pBegin;
        iterator*       m_pEnd;
    };
    //--------------------------------------------------------------------------------------
    template<typename ObjectType>
    BTree<ObjectType>::BTree(PageManager* pm, bool duplicate /*= false*/)
    {

        
        m_PageManager = pm;
        m_PageManager->setHeaderSize(sizeof(TreeInfo));
        cout << sizeof(TreeInfo) << endl;
        initialize();
        m_Info.m_bDuplicate = duplicate;
        if( m_PageManager->getAccessMode() == PageManager::DB_READWRITE )
        {
            Page* page = m_PageManager->getHeaderPage();
            memcpy((void *)&m_Info, (TreeInfo *)(page->getData()), page->getPageSize());
            Page* p = m_PageManager->getPage(0);
            m_Info.m_Order = 9; // Only for 256 string
            m_pRoot = new BPage();
            m_pRoot->unSerialize(p, order());
        }
    }
    
    template<typename ObjectType>
    void BTree<ObjectType>::setPageManager(PageManager* pm, bool duplicate)
    {
        m_PageManager = pm;
        m_PageManager->setHeaderSize(sizeof(TreeInfo));
        
        initialize();
        m_Info.m_bDuplicate = duplicate;
        if( m_PageManager->getAccessMode() == PageManager::DB_READWRITE )
        {
            Page* page = m_PageManager->getHeaderPage();
            memcpy((void *)&m_Info, (TreeInfo *)(page->getData()), page->getPageSize());
            Page* p = m_PageManager->getPage(0);
            m_pRoot = new BPage();
            m_pRoot->unSerialize(p, order());
        }
    }
    
    template<typename ObjectType>
    BTree<ObjectType>::~BTree()
    {
        Page tmpPage(sizeof(TreeInfo));
        memcpy(tmpPage.getData (), &m_Info, sizeof(TreeInfo));
        m_PageManager->writeHeaderPage (&tmpPage);
        delete m_pRoot;
    }
    template<typename ObjectType>
    void BTree<ObjectType>::initialize()
    {
        m_Info.m_Count = 0;
        m_Info.m_Heigth = 0;
        m_pRoot = NULL;
        m_pBegin = NULL;
        m_pEnd = NULL;
    }
    
    template<typename ObjectType>
    void BTree<ObjectType>::insert(const value_type& obj)
    {
        if(!m_pRoot){
            struct PageInfo{
                size_t  m_ItemCount;
                PageID  m_Next;
            };
            m_Info.m_Order = ( m_PageManager->getBufferSize() - sizeof(PageInfo) )/(sizeof(Entry) + obj.m_pObj->serializeSize() ) - 1;
            std::cout << "[DEBUG]" << m_Info.m_Order << "\n";
            m_pRoot = new BPage( getNewPage (), order ());
            assert( m_pRoot->pageID() == 0);
        }
        b_state msg = insert(*m_pRoot, obj, 0);
        if(msg == BT_OVERFLOW){
            splitRoot();
            m_Info.m_Heigth++;
        }
        m_pEnd = NULL;
        m_pBegin= NULL;
    }
    
    template<typename ObjectType>
    typename BTree<ObjectType>::iterator BTree<ObjectType>::find(ObjectType& obj)
    {
        if(!m_pRoot)
            return end();
        
        BPage tmp(*m_pRoot);
        
        for (size_t i = 0; i < m_Info.m_Heigth; i++)
        {
            size_t pos =0;
            while( pos < tmp.count() && *tmp.m_Entries[pos].m_pObj <= obj )
                ++pos;
            
            PageID next = tmp.m_Entries[pos].m_Ptr;
            tmp.clear ();
            tmp.unSerialize(m_PageManager->getPage (next), order ());
        }
        size_t pos =0;// binarySearch(tmp.getEntries(), tmp.count(), obj);
        while( pos < tmp.count() && *tmp.m_Entries[pos].m_pObj < obj )
            ++pos;
        if ( pos < tmp.count () && (*(tmp.m_Entries[pos].m_pObj) == obj) )
            return iterator(tmp, order (), pos, m_PageManager);
        return end();
    }
    template<typename ObjectType>
    typename BTree<ObjectType>::iterator BTree<ObjectType>::findLess(ObjectType& obj)
    {
        if(!m_pRoot)
            return end();
        
        BPage tmp(*m_pRoot);
        for (size_t i = 0; i < m_Info.m_Heigth; i++)
        {
            size_t pos =0;
            while( pos < tmp.count() && *tmp.m_Entries[pos].m_pObj <= obj ) ++pos;
            PageID next = tmp.m_Entries[pos].m_Ptr;
            tmp.clear ();
            tmp.unSerialize(m_PageManager->getPage (next), order ());
        }
        size_t pos =0;
        while( pos < tmp.count() && *tmp.m_Entries[pos].m_pObj < obj ) ++pos;
        
        if ( pos < tmp.count () && (*(tmp.m_Entries[pos].m_pObj) == obj) )
            return iterator(tmp, order (), pos, m_PageManager);
        --pos;
        if ( pos < tmp.count () && (*(tmp.m_Entries[pos].m_pObj) <= obj) )
            return iterator(tmp, order (), pos, m_PageManager);
        return end();
    }
    template<typename ObjectType>
    typename BTree<ObjectType>::iterator BTree<ObjectType>::findMajor(ObjectType& obj)
    {
        if(!m_pRoot)
            return end();
        
        BPage tmp(*m_pRoot);
        for (size_t i = 0; i < m_Info.m_Heigth; i++)
        {
            size_t pos =0;
            while( pos < tmp.count() && *tmp.m_Entries[pos].m_pObj <= obj ) ++pos;
            PageID next = tmp.m_Entries[pos].m_Ptr;
            tmp.clear ();
            tmp.unSerialize(m_PageManager->getPage (next), order ());
        }
        size_t pos =0;
        while( pos < tmp.count() && *tmp.m_Entries[pos].m_pObj < obj ) ++pos;
        if ( pos < tmp.count () && (*(tmp.m_Entries[pos].m_pObj) >= obj) )
            return iterator(tmp, order (), pos, m_PageManager);
        return end();
    }
    
    template<typename ObjectType>
    typename BTree<ObjectType>::b_state BTree<ObjectType>::insert(BPage& ptr, const value_type& obj, size_t level)
    {
        if( level == m_Info.m_Heigth ){//IS lEAF
            size_t pos  = 0;
            while( pos < ptr.count() && (*ptr.m_Entries[pos].m_pObj < *obj.m_pObj) )
                ++pos;
            //size_t pos = binarySearch(ptr.getEntries(), ptr.count(), *obj.m_pObj);
            
            if ( (pos < ptr.count()) && (*ptr.m_Entries[pos].m_pObj == *obj.m_pObj)  )
            {   if( m_Info.m_bDuplicate)
                ptr.insertInLeaf(obj, pos );
                //std::cout << "insert dupllicate\n";
            }
            else
                ptr.insertInLeaf(obj, pos );
            m_PageManager->writePage ( ptr.serialize () );
        }
        else{
            size_t pos  = 0;
            while( pos < ptr.count() && (*ptr.m_Entries[pos].m_pObj <= *obj.m_pObj) )
                ++pos;
            BPage tmp;
            tmp.unSerialize( m_PageManager->getPage ( ptr.getChild(pos) ) , order ());
            b_state msg = insert (tmp, obj, level + 1);
            if ( msg == BT_OVERFLOW )
            {
                split (ptr, tmp, pos, level);
            }
        }
        return isOverflow (ptr)? BT_OVERFLOW : BT_OK;
    }
    
    template<typename ObjectType>
    void    BTree<ObjectType>::splitRoot()
    {
        BPage child(*m_pRoot);
        BPage child0(getNewPage (), order ()),
        child1(getNewPage (), order ());
        
        int i;
        for (i=0; i<child.count ()/2; i++)
            child0.push_back (child.m_Entries[i]);
        child0.m_Entries[i].m_Ptr = child.getChild(i);
        int mid = i;
        if( m_Info.m_Heigth > 0 )    // is index
            ++i;
        int k;
        for (k=0 ; i < child.count (); k++, i++)
            child1.push_back (child.m_Entries[i]);
        child1.getChild(k) = child.getChild(i);
        
        m_pRoot->clear();
        if( m_Info.m_Heigth > 0 )    // is index
            m_pRoot->insertSubTree(child.m_Entries[ mid ].m_pObj, 0);
        else{                       // is leaf
            m_pRoot->insertSubTree(child1.m_Entries[0].m_pObj, 0);
            child0.m_Info.m_Next = child1.pageID();
        }
        m_pRoot->m_Entries[ 0 ].m_Ptr = child0.pageID();
        m_pRoot->m_Entries[ 1 ].m_Ptr = child1.pageID();
        
        m_PageManager->writePage (child0.serialize () );
        m_PageManager->writePage (child1.serialize () );
        m_PageManager->writePage (m_pRoot->serialize ());
    }
    
    template<typename ObjectType>
    void    BTree<ObjectType>::split(BPage& parent, BPage& child, size_t& pos, size_t &level)
    {
        BPage child0(child),
        child1(getNewPage (), order ());
        child0.reset();
        
        int i;
        for (i=0; i<child.count ()/2; i++)
            child0.push_back (child.m_Entries[i]);
        child0.m_Entries[i].m_Ptr = child.getChild(i);
        
        int mid = i;
        if( level+1 != m_Info.m_Heigth ) // index
            i++;
        int k;
        for (k=0 ; i < child.count (); i++, k++)
            child1.push_back (child.m_Entries[i]);
        child1.getChild(k) = child.getChild(i);
        if( level+1 != m_Info.m_Heigth ) // index
            parent.insertSubTree(child.m_Entries[mid].m_pObj, pos);
        else{                           // leaf
            parent.insertSubTree(child1.m_Entries[0].m_pObj, pos);
            child1.m_Info.m_Next = child.m_Info.m_Next;
            child0.m_Info.m_Next = child1.pageID();
        }
        parent.getChild(pos + 1) = child1.pageID();
        
        m_PageManager->writePage (child0.serialize () );
        m_PageManager->writePage (child1.serialize () );
        m_PageManager->writePage (parent.serialize ());
    }
    template<typename ObjectType>
    typename BTree<ObjectType>::iterator  BTree<ObjectType>::begin()
    {
        if( !m_pBegin )
        {
            BPage tmp;
            tmp.unSerialize( m_PageManager->getPage ( 0 ) , order () );
            for (size_t i=0; i<m_Info.m_Heigth;i++)
            {
                PageID ptr = tmp.m_Entries[0].m_Ptr ;
                tmp.clear ();
                tmp.unSerialize (m_PageManager->getPage ( ptr )  , order ());
            }
            m_pBegin = new iterator(tmp, order(), 0, m_PageManager);
        }
        return iterator(*m_pBegin);
    }
    template<typename ObjectType>
    typename BTree<ObjectType>::iterator    BTree<ObjectType>::end()
    {
        if ( !m_pEnd )
        {
            Page* buffer = new Page( m_PageManager->getBufferSize() ,-1);
            BPage tmp(buffer, order());
            tmp.m_Info.m_ItemCount = 0;
            tmp.m_Info.m_Next = 0;
            m_pEnd = new iterator(tmp, order(), -1 , m_PageManager);
        }
        return iterator(*m_pEnd);
    }
    
    template<typename ObjectType>
    void BTree<ObjectType>::print(std::ostream& out)
    {
        out<<"********************\n";
        if(m_pRoot)
            print (*m_pRoot, out, 0);
        out<<"\n********************\n";
    }
    template<typename ObjectType>
    void BTree<ObjectType>::print(BPage& ptr, std::ostream& out, size_t level)
    {
        if( level == m_Info.m_Heigth  )
        {
            for (size_t k=0; k<level ;k++)
                out << "         ";
            out << "(" << ptr.pageID() << "->" << ptr.m_Info.m_Next << ")\n";
        }
        for (int i = ptr.count() - 1; i>=0; i--)
        {
            if( level == m_Info.m_Heigth )
            {
                for (size_t k=0; k<level ;k++)
                    out << "         ";
                out << ptr.m_Entries[i].m_pObj->m_Data << "\n";
            }
            else{
                BPage tmp;
                Page* pag = m_PageManager->getPage( ptr.m_Entries[i+1].m_Ptr );
                tmp.unSerialize(pag, order ());
                print(tmp, out, level + 1);
                for (size_t k=0; k<level ;k++)
                    out << "         ";
                out << ptr.m_Entries[i].m_pObj->m_Data << "\n";
            }
        }
        if( level != m_Info.m_Heigth  )
        {
            BPage tmp;
            Page* pag = m_PageManager->getPage( ptr.m_Entries[0].m_Ptr );
            tmp.unSerialize(pag, order ());
            print(tmp, out, level + 1);
        }
    }
    
    template <typename Conteiner ,typename KeyType>
    int binarySearch(Conteiner Array ,int size ,const KeyType &key)
    {
        
        int i = 0 ,j = size;
        if( i >= j ) return i;
        
        while( i < j )
        {
            int mid = (i+j)/2;
            if( key == *Array[mid] )
                return mid;
            if( key > *Array[mid ] )
                i = mid+1;
            else
                j = mid;
        }
        if( key <= *Array[i-1] )
            return i;
        return j;
        
    }
}