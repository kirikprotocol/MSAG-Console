#ifndef SCAG_UTIL_PROPERTIES
#define SCAG_UTIL_PROPERTIES

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#include <string>

#include <util/Exception.hpp>

namespace scag { namespace util { namespace properties 
{
    const int PTYPE_INT     = 0;
    const int PTYPE_STR     = 1;
    const int PTYPE_BOOL    = 2;
    const int PTYPE_DATE    = 3;

    using smsc::util::Exception;

    class PropertyException : public Exception
    {
    public:

        PropertyException() : Exception() {};
        PropertyException(const char* fmt,...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        virtual ~PropertyException() throw() {};

    };
    class ConvertException : public PropertyException
    {
    static const char* message;
    public:
        
        ConvertException(const char* val, const char* type) 
            : PropertyException(message, val, type) { SMSC_UTIL_EX_FILL(fmt); };
        virtual ~ConvertException() throw() {};
    };
    class ConstantSetException : public PropertyException
    {
    static const char* message;
    public:
        
        ConstantSetException() : PropertyException(message) {};
        ConstantSetException(const char* fmt,...) 
            : PropertyException() { SMSC_UTIL_EX_FILL(fmt); };
        virtual ~ConstantSetException() throw() {};
    };
    
    struct Changeable;
    class Property
    {
    protected:

        int     type;
        bool    sync, constant;

        int64_t     i_val;
        std::string s_val;

        const std::string& convertToStr();
        int64_t convertToInt ();
        bool    convertToBool();
        time_t  convertToDate();
    
    public:

        Property() 
            : type(PTYPE_INT), sync(false), constant(false), i_val(0), s_val("") {};
        Property(const Property& p) 
            : type(p.type), sync(p.sync), constant(p.constant), i_val(p.i_val), s_val(p.s_val) {};
        ~Property(_type) {};

        const std::string& getStr();
        int64_t getInt ();
        bool    getBool();
        time_t  getDate();

        void setStr(const std::string& val);
        void setInt(int64_t val);
        void setBool(bool val);
        void setDate(time_t val);
    };
    
    class NamedProperty : public Property
    {
    private:
    
        std::string name;
        Changeable* patron;

    public:

        NamedProperty(const std::string& _name, Changeable* _patron) 
            : Property(), name(_name), patron(_patron) {};
        NamedProperty(const NamedProperty& p) 
            : Property(p), name(p.name), patron(p.patron) {};

        const std::string& getName() {
            return name;
        }

        void setStr(const std::string& val);
        void setInt(int64_t val);
        void setBool(bool val);
        void setDate(time_t val);
    };

    struct Changeable
    {
        /**
         * Method called on property change to notify it's patron
         * 
         * @param   property    changed property
         */
        virtual void changed(const NamedProperty& property) = 0;
        virtual ~Changeable() {};
        
    protected:
        Changeable() {};
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
        virtual NamedProperty* getProperty(const std::string& name) = 0;
        virtual ~PropertyManager() {};

    protected:
        PropertyManager() {};
    };

}}}

#endif // SCAG_UTIL_PROPERTIES

