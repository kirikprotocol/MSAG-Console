#ifndef _SCAG_UTIL_PROPERTIES_PROPERTIES2_H
#define _SCAG_UTIL_PROPERTIES_PROPERTIES2_H

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif    

#include <string>
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
    typedef std::string string_type;
public:
    Property(): sync(false), type(pt_str), i_val(0) {}
    virtual ~Property() {}

    const string_type& getStr() const;
    int64_t getInt () const;
    bool    getBool() const;
    time_t  getDate() const;
    inline PropertyType getType() const {return type;}

    virtual void setStr( const string_type& val );
    virtual void setInt( int64_t val );
    virtual void setBool( bool val );
    virtual void setDate( time_t val );

    /// virtual string& _setStr();

    int Compare( const string_type& val ) const;
    int Compare( bool val ) const;
    int Compare( int64_t val ) const;
    inline int Compare( int val ) const { return Compare( int64_t(val) ); }
    inline int Compare( unsigned val ) const { return Compare( int64_t(val) ); }
    int Compare( const Property& val, PropertyType pt ) const;
    int Compare( const Property& val, bool reqcast ) const;

protected:
    const std::string& convertToStr() const;
    int64_t convertToInt () const;
    bool    convertToBool() const;
    time_t  convertToDate() const;

protected:
    mutable bool         sync;
    mutable PropertyType type;
    // db: I removed the 'constant' field
    mutable int64_t      i_val;
    mutable std::string  s_val;
};


class AdapterProperty : public Property
{
public:
    AdapterProperty( const std::string& _name,
                     Changeable* _patron,
                     int InitValue ) :
    patron(_patron), name(_name) {i_val = InitValue; type = pt_int;};

    AdapterProperty( const std::string& _name,Changeable* _patron,const std::string& InitValue) :
    patron(_patron), name(_name) {s_val = InitValue; type = pt_str;};

    AdapterProperty(Changeable* _patron) 
        : patron(_patron) { type = pt_int;};

    const std::string& getName() const { return name; }
    // std::string& setName() { return name; };

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
    virtual ~PropertyManager() {};

public:
    PropertyManager() {};
};

}
}
}

#endif // SCAG_UTIL_PROPERTIES

