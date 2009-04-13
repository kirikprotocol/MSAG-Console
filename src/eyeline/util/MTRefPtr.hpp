/* ************************************************************************** *
 * Various reference counting pointer classes for multithread environment.
 * ************************************************************************** */
#ifndef __ELC_UTIL_MTREFPTR_HPP__
#ident "@(#)$Id$"
#define __ELC_UTIL_MTREFPTR_HPP__

#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace util {

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

// ********************************************************************
// Generic object referee abstract class (interface defined)
// NOTE: objects of that class may be allocated only on heap !!
// ********************************************************************
template <class TArg>
class ObjectRefereeAC_T {
protected:
    TArg  *             pObj;
    mutable unsigned    refCount;
    mutable Mutex       lock;

    virtual ~ObjectRefereeAC_T()
    {
        MutexGuard tmp(lock);
        utilizeObj();
    }

    //Utilizes designated object
    virtual void utilizeObj(void) = 0;

public:
    ObjectRefereeAC_T(TArg * use_obj)
        : pObj(use_obj), refCount(0)
    { }

    void Destroy(void) { delete this; }

    //Returns number of references set to old object.
    //NOTE: it's a caller responsibility to check for existing
    //references to designated object prior to changing it !
    unsigned Reset(TArg * use_obj = NULL)
    {
        MutexGuard tmp(lock);
        unsigned i = pObj ? refCount : 0;
        pObj = use_obj;
        refCount = 0;
        return i;
    }

    //Returns pointer to designated object
    TArg * get(void) const
    {
        MutexGuard tmp(lock);
        return pObj;
    }
    //Returns number of references to designated object
    unsigned RefCount(void) const
    {
        MutexGuard tmp(lock);
        return refCount;
    }
    //Increases references counter
    unsigned Ref(void) const
    {
        MutexGuard tmp(lock);
        if (!++refCount)
            --refCount;
        return refCount;
    }
    //Decreases references counter, utilizes designated object
    //if no more references exist
    unsigned UnRef(void)
    {
        MutexGuard tmp(lock);
        if (refCount)
            --refCount;
        if (!refCount)
            utilizeObj();
        return refCount;
    }

    //Returns pointer to designated object
    TArg * operator->() const { return get(); }

    //Creates a new referee for given object with same settings (their
    //definition is out of scope of this class) as this one
    virtual ObjectRefereeAC_T<TArg> * Clone(TArg * use_obj) const = 0;
};

// **********************************************************************
// An object referee that provides only synchronized access to designated
// object in multithread environment (no object utilization).
// **********************************************************************
template <class TArg>
class CREFObjReferee_T : public ObjectRefereeAC_T<TArg> {
protected:
    ~CREFObjReferee_T()
    { }

    void utilizeObj(void) { return; }

public:
    CREFObjReferee_T(TArg * use_obj)
        : ObjectRefereeAC_T<TArg>(use_obj)
    { }

    // ***********************************************
    // * ObjectRefereeAC_T<> interface implementation
    // ***********************************************

    //Creates a new referee for given object
    ObjectRefereeAC_T<TArg> * Clone(TArg * use_obj) const
    {
        return new CREFObjReferee_T(use_obj);
    }
};

// ********************************************************************
// An object referee that provides synchronized access and utilization
// of designated object on last Unref() call in multithread environment
// ********************************************************************
template <class TArg>
class ObjUtilizatorIface_T { //generic object utilizator interface
public:
    virtual void utilize(TArg * use_obj) = 0;
};

template <class TArg>
class UTZObjReferee_T : public ObjectRefereeAC_T<TArg> {
protected:
    ObjUtilizatorIface_T<TArg> *   pUtz;

    void utilize(TArg * use_obj)
    {
        if (pUtz)
            pUtz->utilize(use_obj)
        else
            delete use_obj; 
    }

    ~UTZObjReferee_T()
    { }

public:
    UTZObjReferee_T(TArg * use_obj, ObjUtilizatorIface_T<TArg> * use_utz = NULL)
        : ObjectRefereeAC_T<TArg>(use_obj), pUtz(use_utz)
    { }

    // ***********************************************
    // * ObjectRefereeAC_T<> interface implementation
    // ***********************************************

    //Creates a new referee for given object that uses the same utilizator.
    ObjectRefereeAC_T<TArg> * Clone(TArg * use_obj) const
    {
        return new UTZObjReferee_T(use_obj, pUtz);
    }
};

// **********************************************************************
// Generic reference counting pointer class.  If object of this class
// tracks a last reference to objectReferee it destroys the latter,
// forcing the objectReferee to perform an utilization of designated
// object according to referee implementation specifics.
// **********************************************************************
template <class TArg>
class RFPtr_T {
protected:
    ObjectRefereeAC_T<TArg> *objData;

    void RefObj(void)
    {
        if (objData)
            objData->Ref();
    }
    void UnrefObj(void)
    {
        if (objData && !objData->UnRef()) {
            objData->Destroy(); objData = NULL;
        }
    }

public:
    //Takes ownership of ObjectReferee
    RFPtr_T(ObjectRefereeAC_T<TArg> * use_referee = 0)
        : objData(use_referee)
    {
        RefObj();
    }
    RFPtr_T(const RFPtr_T & src)
    {
        src.RefObj();
        objData = src.objData;
    }
    virtual ~RFPtr_T()
    {
        UnrefObj();
    }

    TArg * get(void) const { return objData ? objData->get() : 0; }
    unsigned  RefCount(void) const { return objData ? objData->RefCount() : 0; }

    RFPtr_T & operator=(ObjectRefereeAC_T<TArg> * use_referee)
    {
        UnrefObj();
        use_referee->Ref();
        objData = use_referee;
        return *this;
    }

    RFPtr_T & operator=(const RFPtr_T & src)
    {
        return (this != &src) ? this->operator=(src.objData) : *this;
    }

    bool operator==(const RFPtr_T & ref2) const
    {
        return (objData == ref2.objData) ? true : false;
    }

    //NOTE: it's a caller responsibility to check for NULL
    //condition prior to accessing the designated object
    TArg * operator->() const { return get(); }
};


// ********************************************************************
// Reference counting pointer class (access and utilization).
// If object of this class tracks a last reference to designated
// object, the latter is utilized (deleted by default).
// ********************************************************************
template <class TArg>
class URFPtr_T : public RFPtr_T<TArg> {
private:
    //hide these operators because of there are own versions
    using RFPtr_T<TArg>::operator=;
    using RFPtr_T<TArg>::operator==;

public:
    URFPtr_T(TArg * use_obj = 0, ObjUtilizatorIface_T<TArg> * use_utz = NULL)
        : RFPtr_T<TArg>(new UTZObjReferee_T<TArg>(use_obj, use_utz))
    { }
    URFPtr_T(const URFPtr_T & src)
    {
        src.RefObj();
        objData = src.objData;
    }
    ~URFPtr_T()
    { }

    URFPtr_T & operator=(UTZObjReferee_T<TArg> * use_referee)
    {
        ((RFPtr_T<TArg>*)this)->operator=(use_referee);
        return *this;
    }

    URFPtr_T & operator=(const URFPtr_T & src)
    {
        return (this != &src) ? this->operator=(src.objData) : *this;
    }

    //NOTE: if URFPtr_T has no objectReferee set, this assignment sets
    //      default object utilization action (call of 'delete')
    URFPtr_T & operator=(TArg * use_obj)
    {
        std::auto_ptr< UTZObjReferee_T<TArg> > newRef(objData ? objData->Clone(use_obj) :
                                                      new UTZObjReferee_T<TArg>(use_obj));
        this->operator=(newRef.get());
        newRef.release();
        return *this;
    }

    bool operator== (const URFPtr_T & ref2) const
    {
        return (objData == ref2.objData) ? true : false;
    }
};

// ********************************************************************
// Reference counting pointer class (access only).
// If object of this class tracks a last reference to designated
// object, its referee is destroyed (not the designated object itself).
// ********************************************************************
template <class TArg>
class CRFPtr_T : public RFPtr_T<TArg> {
private:
    //hide these operators because of there are own versions
    using RFPtr_T<TArg>::operator=;
    using RFPtr_T<TArg>::operator==;

public:
    CRFPtr_T(TArg * use_obj = 0)
        : RFPtr_T<TArg>(new CREFObjReferee_T<TArg>(use_obj))
    { }
    CRFPtr_T(const CRFPtr_T & src)
    {
        src.RefObj();
        objData = src.objData;
    }
    ~CRFPtr_T()
    { }

    CRFPtr_T & operator=(CREFObjReferee_T<TArg> * use_referee)
    {
        ((RFPtr_T<TArg>*)this)->operator=(use_referee);
        return *this;
    }

    CRFPtr_T & operator=(const CRFPtr_T & src)
    {
        return (this != &src) ? this->operator=(src.objData) : *this;
    }

    CRFPtr_T & operator=(TArg * use_obj)
    {
        std::auto_ptr< CREFObjReferee_T<TArg> > newRef(objData ? objData->Clone(use_obj) :
                                                      new CREFObjReferee_T<TArg>(use_obj));
        this->operator=(newRef.get());
        newRef.release();
        return *this;
    }

    bool operator== (const CRFPtr_T & ref2) const
    {
        return (objData == ref2.objData) ? true : false;
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

} //util
} //eyeline

#endif /* __ELC_UTIL_MTREFPTR_HPP__ */

