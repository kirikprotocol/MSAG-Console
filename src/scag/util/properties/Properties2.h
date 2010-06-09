#ifndef _SCAG_UTIL_PROPERTIES_PROPERTIES2_H
#define _SCAG_UTIL_PROPERTIES_PROPERTIES2_H

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif    

#include <time.h>
#include <string>
#include "scag/util/memory/MemoryPoolT.h"
#include "AccessType.h"

namespace scag2 {
namespace util {
namespace properties {

typedef enum PropertyType
{
        pt_int,
        pt_str,
        pt_bool,
        pt_date
} PropertyType;

struct Changeable;

class Property
{
public:
    typedef std::basic_string< char, std::char_traits<char>,
        memory::StdAlloc< char, memory::MemoryPoolT< Property > > >
        string_type;
    // typedef std::string string_type;

public:
    static void* operator new ( size_t sz ) {
        return memory::MemoryPoolT< Property >::Instance().allocate( sz );
    }
    static void operator delete( void* p, size_t sz ) {
        return memory::MemoryPoolT< Property >::Instance().deallocate( p, sz );
    }

    Property(): sync(false), type(pt_str), i_val(0) {}
    Property( int64_t v ) : sync(false), type(pt_int), i_val(v) {}
    Property( const string_type& v ) : sync(false), type(pt_str), s_val(v) {}
    // back compatible version
    Property( const char* v ) : sync(false), type(pt_str), s_val(v) {}
    virtual ~Property() {}

    inline const string_type& getStr() const {
        if (type==pt_str) return s_val;
        else return convertToStr();
    }
    inline int64_t getInt () const {
        if (type==pt_int) return i_val;
        else return convertToInt();
    }
    inline bool getBool() const {
        if (type==pt_bool) return bool(i_val);
        else return convertToBool();
    }
    inline time_t getDate() const {
        if (type==pt_date) return time_t(i_val);
        else return convertToDate();
    }
    inline PropertyType getType() const { return type; }

    virtual void setStr( const string_type& val );
    virtual void setInt( int64_t val );
    virtual void setBool( bool val );
    virtual void setDate( time_t val );

    int Compare( const string_type& val ) const;
    int Compare( bool val ) const;
    int Compare( int64_t val ) const;
    inline int Compare( int val ) const { return Compare( int64_t(val) ); }
    inline int Compare( unsigned val ) const { return Compare( int64_t(val) ); }
    int Compare( const Property& val, PropertyType pt ) const;
    int Compare( const Property& val, bool reqcast ) const;

protected:
    const string_type& convertToStr() const;
    int64_t convertToInt () const;
    bool    convertToBool() const;
    time_t  convertToDate() const;

protected:
    mutable bool         sync;
    mutable PropertyType type;
    // db: I removed the 'constant' field
    mutable int64_t      i_val;
    mutable string_type  s_val;
};


class AdapterProperty : public Property
{
public:
    AdapterProperty( const string_type& _name,
                     Changeable* _patron,
                     int InitValue ) :
    Property(int64_t(InitValue)), patron(_patron), name(_name) {}

    AdapterProperty( const string_type& _name, 
                     Changeable* _patron, 
                     const string_type& InitValue ) :
    Property(InitValue), patron(_patron), name(_name) {}

    AdapterProperty(Changeable* _patron) 
        : patron(_patron) { type = pt_int;};

    // back-compatible versions
    AdapterProperty( const char* theName,
                     Changeable* thePatron,
                     int init ) :
    Property(init), patron(thePatron), name(theName) {}
    AdapterProperty( const char* theName,
                     Changeable* thePatron,
                     const std::string& init ) :
    Property(init.c_str()), patron(thePatron), name(theName) {}

    const string_type& getName() const { return name; }
    virtual void setStr( const string_type& val );
    virtual void setInt( int64_t val );
    virtual void setBool( bool val );
    virtual void setDate( time_t val );

/*        virtual void setPureStr(const std::string& val);
        virtual void setPureInt(int64_t val);
        virtual void setPureBool(bool val);
        virtual void setPureDate(time_t val);       */

private:
    Changeable* patron;
    string_type name;
};


struct Changeable
{
    /**
     * Method called on property change to notify it's patron
     * 
     * @param   property    changed property
     */
    virtual void changed(AdapterProperty& property) = 0;

    virtual ~Changeable() {}
        
protected:
    Changeable() {}
private:
    Changeable( const Changeable& );
    Changeable& operator = ( const Changeable& );
};


struct PropertyManager : public Changeable
{
    /**
     * Returns property for specified name.
     * If manager supports property add then new property returned
     * Othervise null is returned
     * 
     * @param   name        property name
     * @return  property    existed property, new property or null
     */
    virtual Property* getProperty( const std::string& name ) = 0;
    virtual void delProperty( const std::string& name ) = 0;

    virtual ~PropertyManager() {};

public:
    PropertyManager() {};
};

}
}
}

#endif // SCAG_UTIL_PROPERTIES

