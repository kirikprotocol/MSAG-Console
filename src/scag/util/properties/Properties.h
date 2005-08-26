#ifndef SCAG_UTIL_PROPERTIES
#define SCAG_UTIL_PROPERTIES

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif    

#include <string>

namespace scag { namespace util { namespace properties 
{
    enum PropertyType
    {
        pt_int,
        pt_str,
        pt_bool,
        pt_date
    };

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

        Property(): type(pt_str), sync(true), constant(false),i_val(0) {};
        ~Property() {};

        const std::string& getStr();
        int64_t getInt ();
        bool    getBool();
        time_t  getDate();

        virtual void setStr(const std::string& val);
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
            : patron(_patron), name(_name) { i_val = InitValue; type = pt_int;};

        AdapterProperty(const std::string& _name,Changeable* _patron,const std::string& InitValue) 
            : patron(_patron), name(_name) {s_val = InitValue; type = pt_str;};

        virtual void setStr(const std::string& val);
        virtual void setInt(int64_t val);
        virtual void setBool(bool val);
        virtual void setDate(time_t val);
        const std::string& GetName() const {return name;}

        virtual void setPureStr(const std::string& val);
        virtual void setPureInt(int64_t val);
        virtual void setPureBool(bool val);
        virtual void setPureDate(time_t val);

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

}}}

#endif // SCAG_UTIL_PROPERTIES

