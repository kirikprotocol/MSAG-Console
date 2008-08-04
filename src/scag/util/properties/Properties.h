#ifndef SCAG_UTIL_PROPERTIES
#define SCAG_UTIL_PROPERTIES

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif    

#include <string>
#include <time.h>

namespace scag { namespace re {
    typedef enum AccessType 
    {
        atNoAccess = 0,
        atRead  = 1,
        atWrite = 2,
        atReadWrite = 3
    } AccessType;

}};


namespace scag { namespace util { namespace properties 
{
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
    protected:

        PropertyType  type;
        bool       sync, constant;

        int64_t     i_val;
        std::string s_val;

        const std::string& convertToStr();
        int64_t convertToInt ();
        bool    convertToBool();
        time_t  convertToDate();


    public:

        Property(): type(pt_str), sync(false), constant(false),i_val(0) {};
        ~Property() {};

        const std::string& getStr();
        int64_t getInt ();
        bool    getBool();
        time_t  getDate();
        PropertyType getType() {return type;}

        virtual void setStr(const std::string& val);
        virtual std::string& _setStr();
        virtual void setInt(int64_t val);
        virtual void setBool(bool val);
        virtual void setDate(time_t val);

        int Compare(const std::string& val);
        int Compare(bool val);
        int Compare(int val);
        int Compare(Property& val,PropertyType pt);
        int Compare(Property& val,bool reqcast);
    };
    
    class AdapterProperty : public Property
    {
    private:
    
        Changeable* patron;
        std::string name;

    public:

        AdapterProperty(const std::string& _name,Changeable* _patron,int InitValue) 
            : patron(_patron), name(_name) {i_val = InitValue; type = pt_int;};

        AdapterProperty(const std::string& _name,Changeable* _patron,const std::string& InitValue) 
            : patron(_patron), name(_name) {s_val = InitValue; type = pt_str;};

        AdapterProperty(Changeable* _patron) 
            : patron(_patron) { type = pt_int;};

        virtual void setStr(const std::string& val);
        std::string& setName() { return name; };
        virtual void setInt(int64_t val);
        virtual void setBool(bool val);
        virtual void setDate(time_t val);
        const std::string& GetName() const {return name;}

/*        virtual void setPureStr(const std::string& val);
        virtual void setPureInt(int64_t val);
        virtual void setPureBool(bool val);
        virtual void setPureDate(time_t val);       */

    };

    struct Changeable
    {
        /**
         * Method called on property change to notify it's patron
         * 
         * @param   property    changed property
         */
        virtual void changed(AdapterProperty& property) = 0;
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
        virtual Property* getProperty(const std::string& name) = 0;
        virtual ~PropertyManager() {};

    public:
        PropertyManager() {};
    };
 /*
    std::string ConvertStrToWStr(const char * str);
    std::string ConvertWStrToStr(std::string wstr);

    std::string FormatWStr(std::string& wstr);
    std::string UnformatWStr(std::string& str);

    // Functions for converting 2bytes strings to/from standard wstring type
    std::wstring ConvertWStrTo_wstring(const std::string& str);
    std::string Convert_wstringToWStr(const std::wstring& wstr);*/
}}}

#endif // SCAG_UTIL_PROPERTIES

