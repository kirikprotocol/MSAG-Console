/* ************************************************************************** *
 * Various reference counting pointer classes for multithread environment.
 * ************************************************************************** */
#ifndef __MT_REFPTR_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __MT_REFPTR_HPP__

#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace core {
namespace synchronization {
// ********************************************************************
// Generic object referee abstract class (with interface)
// NOTE: objects of that class may be allocated only on heap !!
// ********************************************************************
template <class TArg>
class ObjectRefereeAC_T {
protected:
    TArg  *             pObj;
    mutable unsigned    refCount;

    virtual ~ObjectRefereeAC_T()
    { }

public:
    ObjectRefereeAC_T(TArg * use_obj)
        : pObj(use_obj), refCount(0)
    { }

    virtual void Destroy(void) { delete this; }

    //Returns number of references set to old object.
    //NOTE: it's a caller responsibility to check for existing
    //references to designated object prior to changing it !
    virtual unsigned Reset(TArg * use_obj = NULL)
    {
        unsigned i = pObj ? refCount : 0;
        pObj = use_obj;
        refCount = 0;
        return i;
    }

    //Returns pointer to designated object
    virtual TArg * get(void) const = 0;
    //Returns number of references to designated object
    virtual unsigned RefCount(void) const = 0;
    //Increases references counter
    virtual unsigned Ref(void) const = 0;
    //Decreases references counter, utilizes designated object
    //if no more references exist
    virtual unsigned UnRef(void) = 0;

    //Creates a new referee for given object with same settings as this one
    //(their definition is out of scope of this class)
    virtual ObjectRefereeAC_T<TArg> * Resemble(TArg * use_obj) const = 0;

    //Returns pointer to designated object
    TArg * operator->() const { return get(); }
};

// ********************************************************************
// An object referee that provides synchronized access to designated
// object in multithread environment
// ********************************************************************
template <class TArg>
class MTObjectReferee_T : public ObjectRefereeAC_T<TArg> {
protected:
    mutable Mutex   lock;

    ~MTObjectReferee_T()
    {
        MutexGuard tmp(lock); //synchronize before deletion
    }

public:
    MTObjectReferee_T(TArg * use_obj)
        : ObjectRefereeAC_T<TArg>(use_obj)
    { }

    // ***********************************************
    // * ObjectRefereeAC_T<> interface implementation
    // ***********************************************

    //Returns number of references set to old object.
    //NOTE: it's a caller responsibility to check for existing
    //references to designated object prior to changing it !
    unsigned Reset(TArg * use_obj = NULL)
    {
        MutexGuard tmp(lock);
        return ObjectRefereeAC_T<TArg>::Reset(use_obj);
    }

    //Returns pointer to designated object
    TArg * get(void) const
    {
        MutexGuard tmp(lock);
        return ObjectRefereeAC_T<TArg>::pObj;
    }
    //Returns number of references to designated object
    unsigned  RefCount(void) const
    {
        MutexGuard tmp(lock);
        return ObjectRefereeAC_T<TArg>::refCount;
    }
    //Increases references counter
    unsigned Ref(void) const
    {
        MutexGuard tmp(lock);
        if (!++ObjectRefereeAC_T<TArg>::refCount)
            --ObjectRefereeAC_T<TArg>::refCount;
        return ObjectRefereeAC_T<TArg>::refCount;
    }
    //Decreases references counter, resets pointer to designated
    //object if no more references exist
    unsigned UnRef(void)
    {
        MutexGuard tmp(lock);
        if (ObjectRefereeAC_T<TArg>::refCount)
            --ObjectRefereeAC_T<TArg>::refCount;
        if (!ObjectRefereeAC_T<TArg>::refCount)
            ObjectRefereeAC_T<TArg>::pObj = NULL;
        return ObjectRefereeAC_T<TArg>::refCount;
    }
    //Creates a new referee for given object
    ObjectRefereeAC_T<TArg> * Resemble(TArg * use_obj) const
    {
        return new MTObjectReferee_T(use_obj);
    }
};

// ********************************************************************
// An object referee that provides synchronized access and utilization
// of designated object on last Unref() call in multithread environment
// ********************************************************************
template <class TArg>
class ObjectUtilizatorITF_T { //generic object utilizator interface
protected:
    virtual ~ObjectUtilizatorITF_T() //forbid interface destruction
    { }

public:
    virtual void utilize(TArg *use_obj) = 0;
};

template <class TArg>
class ObjectUtilizatorDFL_T : public ObjectUtilizatorITF_T<TArg> {
public:
    ObjectUtilizatorDFL_T()
    { }
    ~ObjectUtilizatorDFL_T()
    { }

    void utilize(TArg *use_obj) { delete use_obj; }
};

template <class TArg>
class UTZObjectReferee_T : public ObjectRefereeAC_T<TArg> {
protected:
    mutable Mutex   lock;
    ObjectUtilizatorDFL_T<TArg>     dflUtz;
    ObjectUtilizatorITF_T<TArg> *   pUtz;

    ~UTZObjectReferee_T()
    {
        MutexGuard tmp(lock);
        if (ObjectRefereeAC_T<TArg>::pObj)
            pUtz->utilize(ObjectRefereeAC_T<TArg>::pObj);
    }

public:
    UTZObjectReferee_T(TArg * use_obj, ObjectUtilizatorITF_T<TArg> * use_utz = NULL)
        : ObjectRefereeAC_T<TArg>(use_obj)
    { 
        pUtz = use_utz ? use_utz : &dflUtz;
    }

    // ***********************************************
    // * ObjectRefereeAC_T<> interface implementation
    // ***********************************************

    //Returns number of references set to old object.
    //NOTE: it's a caller responsibility to check for existing
    //references to designated object prior to changing it !
    unsigned Reset(TArg * use_obj = NULL)
    {
        MutexGuard tmp(lock);
        return ObjectRefereeAC_T<TArg>::Reset(use_obj);
    }

    //Returns pointer to designated object
    TArg * get(void) const
    {
        MutexGuard tmp(lock);
        return ObjectRefereeAC_T<TArg>::pObj;
    }
    //Returns number of references to designated object
    unsigned  RefCount(void) const
    {
        MutexGuard tmp(lock);
        return ObjectRefereeAC_T<TArg>::refCount;
    }
    //Increases references counter
    unsigned Ref(void) const
    {
        MutexGuard tmp(lock);
        if (!++ObjectRefereeAC_T<TArg>::refCount)
            --ObjectRefereeAC_T<TArg>::refCount;
        return ObjectRefereeAC_T<TArg>::refCount;
    }
    //Decreases references counter, utilizes designated object
    //if no more references exist
    unsigned UnRef(void)
    {
        MutexGuard tmp(lock);
        if (ObjectRefereeAC_T<TArg>::refCount)
            --ObjectRefereeAC_T<TArg>::refCount;
        if (!ObjectRefereeAC_T<TArg>::refCount) {
            if (ObjectRefereeAC_T<TArg>::pObj) {
                pUtz->utilize(ObjectRefereeAC_T<TArg>::pObj);
                ObjectRefereeAC_T<TArg>::pObj = NULL;
            }
        }
        return ObjectRefereeAC_T<TArg>::refCount;
    }
    //Creates a new referee for given object that uses the same utilizator.
    ObjectRefereeAC_T<TArg> * Resemble(TArg * use_obj) const
    {
        return new UTZObjectReferee_T(use_obj, pUtz != &dflUtz ? pUtz : NULL);
    }
};

// **********************************************************************
// Generic reference counting pointer class.  If object of this class
// tracks a last reference to objectReferee it destroys the latter,
// forcing the objectReferee to perform an utilization of designated
// object according to referee implementation specifics.
// **********************************************************************
template <class TArg>
class XRefPtr_T {
protected:
    ObjectRefereeAC_T<TArg> *objData;

    virtual void Unref(void)
    {
        if (!objData->UnRef()) {
            objData->Destroy(); objData = NULL;
        }
    }

public:
    //Takes ownership of ObjectReferee
    XRefPtr_T(ObjectRefereeAC_T<TArg> * use_referee)
        : objData(use_referee)
    {
        objData->Ref();
    }
    XRefPtr_T(const XRefPtr_T & src)
    {
        src.objData->Ref();
        objData = src.objData;
    }
    virtual ~XRefPtr_T()
    {
        Unref();
    }

    TArg* get(void) const { return objData->get(); }
    unsigned  RefCount(void) const { return objData->RefCount(); }

    XRefPtr_T & operator=(ObjectRefereeAC_T<TArg> * use_referee)
    {
        Unref();
        use_referee->Ref();
        objData = use_referee;
        return *this;
    }

    XRefPtr_T & operator=(const XRefPtr_T & src)
    {
        return (this != &src) ? this->operator=(src.objData) : *this;
    }

    XRefPtr_T & operator=(TArg * use_obj)
    {
        std::auto_ptr< ObjectRefereeAC_T<TArg> > newRef(objData->Resemble(use_obj));
        this->operator=(newRef.get());
        newRef.release();
        return *this;
    }

    bool operator==(const XRefPtr_T & ref2) const
    {
        return (objData == ref2.objData) ? true : false;
    }

    //NOTE: it's a caller responsibility to check for NULL
    //condition prior to accessing the designated object
    TArg & operator*() const { return *(objData->get()); }
    TArg * operator->() const { return objData->get(); }
};


// ********************************************************************
// Reference counting pointer class (access and utilization).
// If object of this class tracks a last reference to designated
// object, the latter is utilized (deleted by default).
// ********************************************************************
template <class TArg>
class URefPtr_T : public XRefPtr_T<TArg> {
private:
    //hide these operators because of there are own versions
    using XRefPtr_T<TArg>::operator=;
    using XRefPtr_T<TArg>::operator==;

public:
    URefPtr_T(TArg * use_obj, ObjectUtilizatorITF_T<TArg> * use_utz = NULL)
        : XRefPtr_T<TArg>(new UTZObjectReferee_T<TArg>(use_obj, use_utz))
    { }
    ~URefPtr_T()
    { }

    URefPtr_T & operator=(ObjectRefereeAC_T<TArg> * use_referee)
    {
        ((XRefPtr_T<TArg>*)this)->operator=(use_referee);
        return *this;
    }

    URefPtr_T & operator=(const URefPtr_T & src)
    {
        return (this != &src) ? this->operator=(src.objData) : *this;
    }

    URefPtr_T & operator=(TArg * use_obj)
    {
        ((XRefPtr_T<TArg>*)this)->operator=(use_obj);
        return *this;
    }

    bool operator== (const URefPtr_T & ref2) const
    {
        return (XRefPtr_T<TArg>::objData == ref2.objData) ? true : false;
    }
};

// ********************************************************************
// Reference counting pointer class (access only).
// If object of this class tracks a last reference to designated
// object, its referee is destroyed (not the designated object itself).
// ********************************************************************
template <class TArg>
class MTRefPtr_T : public XRefPtr_T<TArg> {
private:
    //hide these operators because of there are own versions
    using XRefPtr_T<TArg>::operator=;
    using XRefPtr_T<TArg>::operator==;

public:
    MTRefPtr_T(TArg * use_obj)
        : XRefPtr_T<TArg>(new MTObjectReferee_T<TArg>(use_obj))
    { }
    ~MTRefPtr_T()
    { }

    MTRefPtr_T & operator=(ObjectRefereeAC_T<TArg> * use_referee)
    {
        ((XRefPtr_T<TArg>*)this)->operator=(use_referee);
        return *this;
    }

    MTRefPtr_T & operator=(const MTRefPtr_T & src)
    {
        return (this != &src) ? this->operator=(src.objData) : *this;
    }

    MTRefPtr_T & operator=(TArg * use_obj)
    {
        ((XRefPtr_T<TArg>*)this)->operator=(use_obj);
        return *this;
    }

    bool operator== (const MTRefPtr_T & ref2) const
    {
        return (XRefPtr_T<TArg>::objData == ref2.objData) ? true : false;
    }
};

// ********************************************************************
// Ordinary guard for ObjectReferee. Increases reference counter on
// creation or assigning and decreases on destruction.
// ********************************************************************
template <class TArg>
class OBJRefGuard_T {
protected:
    ObjectRefereeAC_T<TArg> * objData;

    void Unref(void) { if (objData) objData->UnRef(); }
    void Ref(void)   { if (objData) objData->Ref(); }
public:
    OBJRefGuard_T(ObjectRefereeAC_T<TArg> * use_referee)
        : objData(use_referee)
    {
        Ref();
    }
    ~OBJRefGuard_T()
    {
        Unref();
    }

    TArg * get(void) const { return objData ? objData->get() : NULL; }
    TArg * operator->() const { return get(); }

    OBJRefGuard_T & operator=(ObjectRefereeAC_T<TArg> * use_referee)
    {
        Unref();
        objData = use_referee;
        Ref();
        return *this;
    }
};

}//namespace synchronization
}//namespace core
}//namespace smsc

#endif /* __MT_REFPTR_HPP__ */

