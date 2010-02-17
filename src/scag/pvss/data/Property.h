/* $Id$ */

#ifndef SCAG_PVSS_DATA_PROPERTY
#define SCAG_PVSS_DATA_PROPERTY

#include <list>
#include <string>

#include "scag/util/storage/SerialBuffer.h"
#include "scag/util/io/GlossaryBase.h"
#include "scag/exc/IOException.h"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {

namespace perstypes {

enum TimePolicy{
    UNKNOWN = 0,
    INFINIT,
    FIXED,
    ACCESS,
    R_ACCESS,
    W_ACCESS
};

const char* timePolicyToString( TimePolicy tp );
inline bool isTimePolicyValid( TimePolicy tp ) {
    return (INFINIT <= tp && tp <= W_ACCESS);
}


/*
static const char* TimePolicyStr[] = {
  " TIME_POLICY: UNKNOWN",
  " TIME_POLICY: INFINIT",
  " TIME_POLICY: FIXED",
  " TIME_POLICY: ACCESS",
  " TIME_POLICY: R_ACCESS",
  " TIME_POLICY: W_ACCESS"
};
 */

enum PropertyType {
    INT = 1,
    STRING,
    BOOL,
    DATE
};

const char* propertyTypeToString( PropertyType pt );
/*
{
#define PROPTYPETOSTR(x) case (x) : return #x
    switch (pt) {
        PROPTYPETOSTR(INT);
        PROPTYPETOSTR(STRING);
        PROPTYPETOSTR(BOOL);
        PROPTYPETOSTR(DATE);
#undef PROPTYPETOSTR
    default : return "???";
    }
}
 */

} // namespace perstypes
    
using namespace perstypes;

// static const size_t STRBUF_SIZE = 64;

class Property 
{
private:
    static smsc::logger::Logger* log_;
    static const std::string emptyString_;

    static void initLog();

protected:
    PropertyType type;
    std::string name;
    std::string s_val;
    int64_t i_val;
    time_t d_val;
    bool b_val;

    TimePolicy time_policy;
    time_t final_date;
    uint32_t life_time;

    void copy(const Property& cp);
    static void StringFromBuf( util::storage::SerialBuffer& buf, std::string &str );

    void setPropertyName(const char* nm);

    mutable std::string propertyStr_;  // a cache of property toString() method, empty when not filled.
    // mutable char strBuf[STRBUF_SIZE];

public:

    Property() : type(PropertyType(0)), i_val(0), time_policy(INFINIT) {
        // if (!log_) initLog();
        // smsc_log_debug(log_,"ctor %p",this);
    }
    Property(const Property& cp);
    Property& operator=(const Property& cp);

    Property(const char *nm, int32_t i, TimePolicy policy, time_t fd, uint32_t lt)
    {
        // if (!log_) initLog();
        setInt(nm, i, policy, fd, lt);
        // smsc_log_debug(log_,"ctor %p %s",this,toString().c_str());
    };

    Property(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt)
    {
        // if (!log_) initLog();
        setString(nm, str, policy, fd, lt);
        // smsc_log_debug(log_,"ctor %p %s",this,toString().c_str());
    };

    ~Property() {
        // smsc_log_debug(log_,"dtor %p %s",this,toString().c_str());
    }

    bool operator == ( const Property& prop ) const {
        if ( type != prop.type ||
             name != prop.name || 
             time_policy != prop.time_policy ||
             final_date != prop.final_date ||
             life_time != prop.life_time ) return false;

        bool valueEqual = 
            ( type == STRING ? 
              ( s_val == prop.s_val ) :
              ( type == INT ?
                ( i_val == prop.i_val ) :
                ( type == BOOL ?
                  ( b_val == prop.b_val ) :
                  ( d_val == prop.d_val ) ) ) );
        return valueEqual;
    }
    
    inline bool operator != ( const Property& prop ) const {
        return ! (*this == prop );
    }

    
    void ReadAccess();
    void WriteAccess();

    const char* getName() const { return name.c_str(); };
    void setName(const std::string& nm) { setPropertyName(nm.c_str()); };
    int64_t getIntValue() const { return i_val; };
    bool getBoolValue() const { return b_val; };
    time_t getDateValue() const { return d_val; };
    const std::string& getStringValue() const { return s_val; };
    void setValue(const Property& cp);
    void setValue(const char* str);
    void setIntValue(int64_t i) { invalidateCache(); i_val = i; type = INT; };
    void setBoolValue(bool b) { invalidateCache(); b_val = b; type = BOOL; };
    void setDateValue(time_t d) { invalidateCache(); d_val = d; type = DATE; };
    void setStringValue(const char* s) { invalidateCache(); s_val = s; type = STRING; };
    inline uint8_t getType() const { return type; };
    void setTimePolicy(TimePolicy policy, time_t fd, uint32_t lt);
    TimePolicy getTimePolicy() const { return time_policy; };
    time_t getFinalDate() const { return final_date; }
    uint32_t getLifeTime() const { return life_time; }

    bool isExpired() const;
    bool isExpired(time_t cur_time) const;
    const std::string& toString() const;
    void fromString( const std::string& ) /* throw (exceptions::IOException) */ ;

    void setInt(const char *nm, int32_t i, TimePolicy policy, time_t fd, uint32_t lt);
    void setBool(const char *nm, bool b, TimePolicy policy, time_t fd, uint32_t lt);
    void setString(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt);
    void setDate(const char *nm, time_t t, TimePolicy policy, time_t fd, uint32_t lt);

    void assign(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt);

    void Serialize(util::storage::SerialBuffer& buf, bool fromFSDB = false, util::io::GlossaryBase* glossary = NULL) const;
    void Deserialize(util::storage::SerialBuffer& buf, bool toFSDB = false, util::io::GlossaryBase* glossary = NULL);

    bool convertToInt();

    bool isValid() const {
        return getType() != 0 && isTimePolicyValid(time_policy);
    }

protected:
    inline void invalidateCache() { propertyStr_.clear(); }

};

}//pvss
}//scag2

namespace scag {
namespace pvss {

using scag2::pvss::Property;

}
}

#endif
