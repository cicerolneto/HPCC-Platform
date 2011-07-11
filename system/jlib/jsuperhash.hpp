/*##############################################################################

    Copyright (C) 2011 HPCC Systems.

    All rights reserved. This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
############################################################################## */



#ifndef SUPERHASH_HPP
#define SUPERHASH_HPP

//#define TRACE_HASH
#undef expand

#include "jexpdef.hpp"
#include "jiface.hpp"
#include "jiter.hpp"
#include "jstring.hpp"
#include "jmutex.hpp"

extern jlib_decl unsigned hashc( const unsigned char *k, unsigned length, unsigned initval);
extern jlib_decl unsigned hashnc( const unsigned char *k, unsigned length, unsigned initval);

class jlib_decl SuperHashTable : public CInterface
{
public:
    SuperHashTable(void);
    SuperHashTable(unsigned initsize);
    ~SuperHashTable();
    // Derived class destructor expected to call releaseAll()
    
    void             reinit(unsigned initsize);
    void             kill(void);
    inline unsigned  count() const { return tablecount; }
    inline memsize_t queryMem() const { return tablesize * sizeof(void *); } // hash table table memory size
    void *           next(const void *et) const;

#ifdef TRACE_HASH
    void dumpStats();
#endif

protected:
    void             init(unsigned initsize);
    void             releaseAll(void);
    inline bool      add(void * et) { return doAdd(et, false); }
    inline bool      replace(void * et) { return doAdd(et, true); }
    void             addNew(void * et); //use this when you are sure the key does not already exist in the table (saves some needless matching)
    void             addNew(void * donor, unsigned hash);
    void *           addOrFind(void *);
    inline void *    find(const void * param) const { return table[doFind(param)]; }
    inline void *    find(unsigned hashcode, const void * param) const { return table[doFind(hashcode, param)]; }
    void *           findElement(unsigned hashcode, const void * searchE) const;
    void *           findElement(const void * searchE) const;
    void *           findExact(const void * et) const;
    // remove(void *) zaps key (i.e. a match using matchesFindParam method)
    // removeExact(void *) zaps element (i.e. a match using pointer==pointer)
    bool             remove(const void * param);
    bool             removeExact(void * et);
    inline void      setCache(unsigned v) const { cache = v; }

private:
    bool             doAdd(void *, bool);
    void             doDeleteElement(unsigned);
    inline unsigned  doFind(const void * findParam) const
      { return doFind(getHashFromFindParam(findParam), findParam); }
    unsigned         doFind(unsigned, const void *) const;
    unsigned         doFindElement(unsigned, const void *) const;
    unsigned         doFindNew(unsigned) const;
    unsigned         doFindExact(const void *) const;
    void             doKill(void);
    void             expand();
    void             note_searchlen(int) const;

    virtual void     onAdd(void *et) = 0;
    virtual void     onRemove(void *et) = 0;
    virtual unsigned getHashFromElement(const void *et) const = 0;
    virtual unsigned getHashFromFindParam(const void *fp) const = 0;
    virtual const void * getFindParam(const void *et) const = 0;
    virtual unsigned getTableLimit(unsigned max);
    virtual bool     matchesFindParam(const void *et, const void *key, unsigned fphash) const = 0;
    virtual bool     matchesElement(const void *et, const void *searchET) const;

protected:
    mutable unsigned cache;             // before pointer to improve 64bit packing.
    void * *         table;
    unsigned         tablesize;
    unsigned         tablecount;
#ifdef TRACE_HASH
    mutable int      search_tot;
    mutable int      search_num;
#endif
};

template <class ET, class FP>
class SuperHashTableOf : public SuperHashTable
{
  public:
    SuperHashTableOf(void) : SuperHashTable() {}
    SuperHashTableOf(unsigned initsize) : SuperHashTable(initsize) {}
    inline bool      add(ET & et)
      { return SuperHashTable::add(&et); }
    inline bool      replace(ET & et)
      { return SuperHashTable::replace(&et); }
    inline ET *      addOrFind(ET & et)
      { return static_cast<ET *>(SuperHashTable::addOrFind(&et)); }
    inline ET *      find(const FP * fp) const
      { return static_cast<ET *>(SuperHashTable::find(fp)); }
    inline ET *      find(unsigned hashCode, const FP * fp) const
      { return static_cast<ET *>(SuperHashTable::find(hashCode, fp)); }
    inline ET *      findExact(const ET & et) const
      { return static_cast<ET *>(SuperHashTable::findExact(&et)); }
    inline bool      remove(const FP * fp)
      { return SuperHashTable::remove((const void *)(fp)); }
    inline bool      removeExact(ET * et)
      { return SuperHashTable::removeExact(et); }
};

// Macro to provice find method taking reference instead of pointer

#define IMPLEMENT_SUPERHASHTABLEOF_REF_FIND(ET, FP)                        \
    inline ET *      find(FP & fp) const                                   \
      { return SuperHashTableOf<ET, FP>::find(&fp); }


// simple type hashing HT impl.
// elements (ET) must implement queryFindParam.
template <class ET, class FP>
class SimpleHashTableOf : public SuperHashTableOf<ET, FP>
{
    typedef SimpleHashTableOf<ET, FP> SELF;
public:
    SimpleHashTableOf<ET, FP>(void) : SuperHashTableOf<ET, FP>() { }
    SimpleHashTableOf<ET, FP>(unsigned initsize) : SuperHashTableOf<ET, FP>(initsize) { }
    ~SimpleHashTableOf<ET, FP>() { SELF::kill (); }

    IMPLEMENT_SUPERHASHTABLEOF_REF_FIND(ET, FP);

    virtual void onAdd(void *et) { }
    virtual void onRemove(void *et) { }
    virtual unsigned getHashFromElement(const void *et) const
    {
        return hashc((const unsigned char *) ((const ET *) et)->queryFindParam(), sizeof(FP), 0);
    }
    virtual unsigned getHashFromFindParam(const void *fp) const
    {
        return hashc((const unsigned char *) fp, sizeof(FP), 0);
    }
    virtual const void *getFindParam(const void *et) const
    {
        return ((const ET *)et)->queryFindParam();
    }
    virtual bool matchesFindParam(const void *et, const void *fp, unsigned fphash) const
    {
        return *(FP *)((const ET *)et)->queryFindParam() == *(FP *)fp;
    }
};

template <class ET, class FP>
class OwningSimpleHashTableOf : public SimpleHashTableOf<ET, FP>
{
    typedef OwningSimpleHashTableOf<ET, FP> SELF;
public:
    OwningSimpleHashTableOf<ET, FP>(void) : SimpleHashTableOf<ET, FP>() { }
    OwningSimpleHashTableOf<ET, FP>(unsigned initsize) : SimpleHashTableOf<ET, FP>(initsize) { }
    ~OwningSimpleHashTableOf<ET, FP>() { SELF::kill(); }

    virtual void onRemove(void *et) { ((ET *)et)->Release(); }
};

class jlib_decl SuperHashIterator : public CInterface
{
public:
    SuperHashIterator(const SuperHashTable & _table, bool _linkTable=true) : table(_table), linkTable(_linkTable) { cur = NULL; if (linkTable) table.Link(); }
    ~SuperHashIterator() { if (linkTable) table.Release(); }

    IMPLEMENT_IINTERFACE

    virtual bool     first(void)
      { cur = table.next(NULL); return cur != NULL; }
    virtual bool     isValid(void) { return cur != NULL; }
    virtual bool     next(void)
      { if (cur) cur = table.next(cur); return (cur != NULL); }
 
protected:
    void *           queryPointer() { assertex(cur); return cur; }

private:
    bool linkTable;
    const SuperHashTable & table;
    void*            cur;
};

template <class ET>
class SuperHashIteratorOf : public SuperHashIterator
{
  public:
    SuperHashIteratorOf(const SuperHashTable & _table, bool linkTable=true) : SuperHashIterator(_table, linkTable) {}
    ET &             query()
      { return *(static_cast<ET *>(queryPointer())); }
};

template <class ET>
class StringSuperHashTableOf : public SuperHashTableOf<ET, const char>
{
    typedef StringSuperHashTableOf<ET> SELF;
public:
    StringSuperHashTableOf<ET>(void) : SuperHashTableOf<ET, const char>() { }
    StringSuperHashTableOf<ET>(unsigned initsize) : SuperHashTableOf<ET, const char>(initsize) { }
    ~StringSuperHashTableOf<ET>() { SELF::kill(); }

    virtual void onAdd(void *et) { }
    virtual void onRemove(void *et) { }
    virtual unsigned getHashFromElement(const void *et) const
    {
        const char *str = ((const ET *) et)->queryFindString();
        return hashc((const unsigned char *) str, (size32_t)strlen(str), 0);
    }
    virtual unsigned getHashFromFindParam(const void *fp) const
    {
        return hashc((const unsigned char *) fp, (size32_t)strlen((const char *)fp), 0);
    }
    virtual const void *getFindParam(const void *et) const
    {
        return ((const ET *)et)->queryFindString();
    }
    virtual bool matchesFindParam(const void *et, const void *fp, unsigned fphash) const
    {
        return (0==strcmp(((const ET *)et)->queryFindString(), (const char *)fp));
    }
};

template <class ET>
class OwningStringSuperHashTableOf : public StringSuperHashTableOf<ET>
{
    typedef OwningStringSuperHashTableOf<ET> SELF;
public:
    OwningStringSuperHashTableOf<ET>(void) : StringSuperHashTableOf<ET>() { }
    OwningStringSuperHashTableOf<ET>(unsigned initsize) : StringSuperHashTableOf<ET>(initsize) { }
    ~OwningStringSuperHashTableOf<ET>() { SELF::kill(); }

    virtual void onRemove(void *et) { ((ET *)et)->Release(); }
};

// thread safe simple hash table impl. 
template <class ET, class FP>
class ThreadSafeSimpleHashTableOf : private SuperHashTable
{
    typedef ThreadSafeSimpleHashTableOf<ET, FP> SELF;

    virtual void onAdd(void *et) { }
    virtual void onRemove(void *et) { }
    virtual unsigned getHashFromElement(const void *et) const
    {
        return hashc((const unsigned char *) ((const ET *) et)->queryFindParam(), sizeof(FP), 0);
    }
    virtual unsigned getHashFromFindParam(const void *fp) const
    {
        return hashc((const unsigned char *) fp, sizeof(FP), 0);
    }
    virtual const void *getFindParam(const void *et) const
    {
        return ((const ET *)et)->queryFindParam();
    }
    virtual bool matchesFindParam(const void *et, const void *fp, unsigned fphash) const
    {
        return *(FP *)((const ET *)et)->queryFindParam() == *(FP *)fp;
    }
public:
    mutable CriticalSection crit;

    ThreadSafeSimpleHashTableOf(void) : SuperHashTable() { }
    ThreadSafeSimpleHashTableOf(unsigned initsize) : SuperHashTable(initsize) { }
    ~ThreadSafeSimpleHashTableOf() { SELF::kill(); }

    ET *find(FP & fp) const
    {
        CriticalBlock block(crit);
        return (ET *) SuperHashTable::find(&fp);
    }

    SuperHashTable &queryBaseTable() { return *this; }
    void kill()
    {
        CriticalBlock block(crit);
        SuperHashTable::kill();
    }

    bool add(ET & et)
    { 
        CriticalBlock block(crit);
        return SuperHashTable::add(&et); 
    }
    bool replace(ET & et)
    { 
        CriticalBlock block(crit);
        return SuperHashTable::replace(&et); 
    }
    ET * addOrFind(ET & et)
    { 
        CriticalBlock block(crit);
        return static_cast<ET *>(SuperHashTable::addOrFind(&et)); 
    }
    ET * find(const FP * fp) const
    { 
        CriticalBlock block(crit);
        return static_cast<ET *>(SuperHashTable::find(fp)); 
    }
    ET * findExact(const ET & et) const
    { 
        CriticalBlock block(crit);
        return static_cast<ET *>(SuperHashTable::findExact(&et)); 
    }
    bool remove(const FP * fp)
    { 
        CriticalBlock block(crit);
        return SuperHashTable::remove((const void *)(fp)); 
    }
    bool removeExact(ET * et)
    { 
        CriticalBlock block(crit);
        return SuperHashTable::removeExact(et); 
    }
    unsigned count() const
    {
        CriticalBlock block(crit);
        return SuperHashTable::count();
    }
};

template <class ET, class FP>
class ThreadSafeOwningSimpleHashTableOf : public ThreadSafeSimpleHashTableOf<ET, FP>
{
    typedef ThreadSafeOwningSimpleHashTableOf<ET, FP> SELF;
public:
    ~ThreadSafeOwningSimpleHashTableOf<ET, FP>() { SELF::kill(); }
    virtual void onRemove(void *et) { ((ET *)et)->Release(); }
};

// template mapping object for base type to arbitary object
template <class ET, class FP>
class HTMapping : public CInterface
{
protected:
    ET &et;
    FP fp;
public:
    HTMapping(ET &_et, const FP &_fp) : et(_et), fp(_fp) { }
    const void *queryFindParam() const { return &fp; }
    ET &queryElement() const { return et; }
    FP &queryFindValue() const { return fp; }
};

// template mapping object for base type to IInterface object
template <class ET, class FP>
class LinkedHTMapping : public HTMapping<ET, FP>
{
public:
    LinkedHTMapping(ET &et, FP &fp) : HTMapping<ET, FP>(et, fp) { this->et.Link(); }
    ~LinkedHTMapping() { this->et.Release(); }
};

// template mapping object for string to arbitary object
template <class ET>
class StringHTMapping : public CInterface
{
public:
    StringHTMapping(const char *_fp, ET &_et) : fp(_fp), et(_et) { }
    const char *queryFindString() const { return fp; }

protected:
    ET &et;
    StringAttr fp;
};

// template mapping object for string to IInterface object
template <class ET>
class OwningStringHTMapping : public StringHTMapping<ET>
{
public:
    OwningStringHTMapping(const char *fp, ET &et) : StringHTMapping<ET>(fp, et) { }
    ~OwningStringHTMapping() { this->et.Release(); }
    ET &query() { return this->et; }
    ET &get() { this->et.Link(); return this->et; }
};

// NB: only really here, because of circular include problems

// utility atom class, holding hash,string and count
class jlib_decl HashKeyElement
{
public:
    const char *get() { return (const char *)keyPtr(); }
    unsigned length() { return (size32_t)strlen((const char *)keyPtr()); }
    unsigned queryHash() const { return hashValue; }
    operator const char * () { return (NULL==this)?NULL:keyPtr(); }
    unsigned queryReferences() { return linkCount+1; } // 1 implicit

private:        
    const char *keyPtr() { return ((const char *)this)+sizeof(*this); }

    unsigned hashValue;
    unsigned linkCount;
    HashKeyElement();

friend class AtomRefTable;
};

#ifdef _MSC_VER
#pragma warning(disable : 4275 )
#endif
typedef const char constcharptr;
class jlib_decl AtomRefTable : public SuperHashTableOf<HashKeyElement, constcharptr>
{
    CriticalSection crit;
    bool nocase;

    inline HashKeyElement *createKeyElement(const char *key)
    {
        size32_t l = (size32_t)strlen(key);
        HashKeyElement *hke = (HashKeyElement *) checked_malloc(sizeof(HashKeyElement)+l+1,-605);
        memcpy((void *) (hke->keyPtr()), key, l+1);
        if (nocase)
            hke->hashValue = hashnc((const unsigned char *)key, l, 0);
        else
            hke->hashValue = hashc((const unsigned char *)key, l, 0);
        hke->linkCount = 0;
        verifyex(add(*hke));
        return hke;
    }

public:
    IMPLEMENT_SUPERHASHTABLEOF_REF_FIND(HashKeyElement, constcharptr);

    AtomRefTable(bool _nocase=false) : SuperHashTableOf<HashKeyElement, constcharptr>(3000), nocase(_nocase) { }
    ~AtomRefTable() { kill(); }

    inline HashKeyElement *findLink(const char *_key)
    {
        CriticalBlock b(crit);
        HashKeyElement *key = find(*_key);
        if (key) key->linkCount++;
        return key;
    }

    // assumes key is member of this table.

    inline HashKeyElement *queryCreate(const char *_key)
    {
        CriticalBlock b(crit);
        HashKeyElement *key = find(*_key);
        if (key)
            key->linkCount++;
        else
            key = createKeyElement(_key);
        return key;
    }

    inline void linkKey(const char *key)
    {
        queryCreate(key);
    }

    inline void releaseKey(HashKeyElement *key)
    {
        CriticalBlock b(crit);
        if (0 == key->linkCount)
        {
            verifyex(removeExact(key));
            return;
        }
        --key->linkCount;
    }

protected:
// SuperHashTable definitions
    virtual void onAdd(void *e) { }
    virtual void onRemove(void *e) 
    { 
        free(e); 
    }

    virtual unsigned getHashFromElement(const void *e) const
    {
        return ((HashKeyElement *) e)->queryHash();
    }

    virtual unsigned getHashFromFindParam(const void *fp) const
    {
        if (nocase)
            return hashnc((const unsigned char *)fp, (size32_t)strlen((const char *)fp), 0);
        else
            return hashc((const unsigned char *)fp, (size32_t)strlen((const char *)fp), 0);
    }

    virtual const void *getFindParam(const void *e) const
    {
        return ((HashKeyElement *) e)->get();
    }

    virtual bool matchesFindParam(const void *e, const void *fp, unsigned fphash) const
    {
        if (nocase)
            return (0 == stricmp(((HashKeyElement *)e)->get(), (const char *)fp));
        else
            return (0 == strcmp(((HashKeyElement *)e)->get(), (const char *)fp));
    }
};

#endif
