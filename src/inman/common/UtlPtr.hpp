/* ************************************************************************* *
 * Helper class for automatic object utilizing (analog of std:auto_ptr).
 * ************************************************************************* */
#ifndef __SMSC_UTIL_AUTO_PTR_UTL_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_AUTO_PTR_UTL_HPP

namespace smsc {
namespace util {

class UtilizableObjITF {
protected:
    virtual ~UtilizableObjITF() //forbid interface destruction
    { }

public:
    virtual void Utilize(void) = 0;
};

//std::auto_ptr<> adopted for dealing with UtilizableObjITF
template <class _TArg /* : public UtilizableObjITF */>
class auto_ptr_utl {
private:
    _TArg * _ptr;

public:
    explicit auto_ptr_utl(_TArg * use_ptr = 0) throw()
        : _ptr(use_ptr)
    { }
    auto_ptr_utl(auto_ptr_utl & use_org) throw()
        : _ptr(use_org.release())
    { }
    ~auto_ptr_utl()
    {
        reset(0);
    }

    _TArg * get(void) throw()
    {
        return _ptr;
    }
    _TArg * release(void) throw()
    {
        _TArg * pt = _ptr;
        _ptr = 0;
        return pt;
    }
    void reset(_TArg * use_ptr = 0) //may throw !!!
    {
        if (_ptr != use_ptr) {
            _TArg * pt = _ptr;
            _ptr = use_ptr;
            pt->Utilize();
        }
    }

    _TArg * operator->() const throw()
    {
        return get(); 
    }
    auto_ptr_utl & operator=(auto_ptr_utl & use_org) //may throw !!!
    {
        reset(use_org.release());
        return *this;
    }
};

} //util
} //smsc
#endif /* __SMSC_UTIL_AUTO_PTR_UTL_HPP */

