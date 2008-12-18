/* $Id$ */

#ifndef SCAG_PERS_STORE_PROPERTY
#define SCAG_PERS_STORE_PROPERTY

#include <list>
#include <string>

#include "scag/util/storage/SerialBuffer.h"
#include "scag/util/storage/GlossaryBase.h"

namespace scag { namespace pers { namespace util {

using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;
using scag::util::storage::GlossaryBase;

namespace perstypes {

enum TimePolicy{
    UNKNOWN,
    INFINIT,
    FIXED,
    ACCESS,
    R_ACCESS,
    W_ACCESS
};

static const char* TimePolicyStr[] = {
  " TIME_POLICY: UNKNOWN",
  " TIME_POLICY: INFINIT",
  " TIME_POLICY: FIXED",
  " TIME_POLICY: ACCESS",
  " TIME_POLICY: R_ACCESS",
  " TIME_POLICY: W_ACCESS"
};

enum PropertyType{
    INT = 1,
    STRING,
    BOOL,
    DATE
};

}
    
using namespace perstypes;

static const size_t STRBUF_SIZE = 64;

class Property {
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
    static void StringFromBuf(SerialBuffer& buf, std::string &str);

    void setPropertyName(const char* nm);

    std::string propertyStr;
    char strBuf[STRBUF_SIZE];

public:

    Property() : type(INT), i_val(0), time_policy(INFINIT){};
    Property(const Property& cp);
    Property& operator=(const Property& cp);

    Property(const char *nm, int32_t i, TimePolicy policy, time_t fd, uint32_t lt)
    {
        setInt(nm, i, policy, fd, lt);
    };

    Property(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt)
    {
        setString(nm, str, policy, fd, lt);
    };

    void ReadAccess();
    void WriteAccess();

    const char* getName() const { return name.c_str(); };
    void setName(const std::string& nm) { name = nm; };
    int64_t getIntValue() { return i_val; };
    bool getBoolValue() { return b_val; };
    time_t getDateValue() { return d_val; };
    const std::string& getStringValue() { return s_val; };
    void setValue(const Property& cp);
    void setValue(const char* str);
    void setIntValue(int64_t i) { i_val = i; type = INT; };
    void setBoolValue(bool b) { b_val = b; type = BOOL; };
    void setDateValue(time_t d) { d_val = d; type = DATE; };
    void setStringValue(const char* s) { s_val = s; type = STRING; };
    uint8_t getType() { return type; };
    void setTimePolicy(TimePolicy policy, time_t fd, uint32_t lt);
    TimePolicy getTimePolicy() { return time_policy; };
    bool isExpired();
	bool isExpired(time_t cur_time);
    const std::string& toString();

    void setInt(const char *nm, int32_t i, TimePolicy policy, time_t fd, uint32_t lt);
    void setBool(const char *nm, bool b, TimePolicy policy, time_t fd, uint32_t lt);
    void setString(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt);
    void setDate(const char *nm, time_t t, TimePolicy policy, time_t fd, uint32_t lt);

    void assign(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt);

    void Serialize(SerialBuffer& buf, bool fromFSDB = false, GlossaryBase* glossary = NULL) const;
    void Deserialize(SerialBuffer& buf, bool toFSDB = false, GlossaryBase* glossary = NULL);

    bool convertToInt();
};

}//util
}//pers
}//scag

namespace scag2 {
namespace pers {
namespace util {

using namespace scag::pers::util::perstypes;
using scag::pers::util::Property;

}
}
}

#endif
