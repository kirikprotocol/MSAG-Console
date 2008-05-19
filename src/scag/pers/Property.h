/* $Id$ */

#ifndef SCAG_PERS_STORE_PROPERTY
#define SCAG_PERS_STORE_PROPERTY

#include <list>
#include <string>

#include "SerialBuffer.h"
#include "Glossary.h"

namespace scag{ namespace pers{

//using namespace smsc::core::buffers;

enum TimePolicy{
    UNKNOWN,
    INFINIT,
    FIXED,
    ACCESS,
    R_ACCESS,
    W_ACCESS
};

enum PropertyType{
    INT = 1,
    STRING,
    BOOL,
    DATE
};

class Property {
protected:
    PropertyType type;
    std::string name;
    std::string s_val;
    int32_t i_val;
    time_t d_val;
    bool b_val;

    TimePolicy time_policy;
    time_t final_date;
    uint32_t life_time;

    void copy(const Property& cp);
    static void StringFromBuf(SerialBuffer& buf, std::string &str);

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
    uint32_t getIntValue() { return i_val; };
    bool getBoolValue() { return b_val; };
    time_t getDateValue() { return d_val; };
    const std::string& getStringValue() { return s_val; };
    void setValue(const Property& cp);
    void setValue(const char* str);
    void setIntValue(int32_t i) { i_val = i; type = INT; };
    void setBoolValue(bool b) { b_val = b; type = BOOL; };
    void setDateValue(time_t d) { d_val = d; type = DATE; };
    void setStringValue(const char* s) { s_val = s; type = STRING; };
    uint8_t getType() { return type; };
    void setTimePolicy(TimePolicy policy, time_t fd, uint32_t lt);
    TimePolicy getTimePolicy() { return time_policy; };
    bool isExpired();
	bool isExpired(time_t cur_time);
    std::string toString() const;

    void setInt(const char *nm, int32_t i, TimePolicy policy, time_t fd, uint32_t lt);
    void setBool(const char *nm, bool b, TimePolicy policy, time_t fd, uint32_t lt);
    void setString(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt);
    void setDate(const char *nm, time_t t, TimePolicy policy, time_t fd, uint32_t lt);

    void assign(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt);

    void Serialize(SerialBuffer& buf, bool fromFSDB = false) const;
    void Deserialize(SerialBuffer& buf, bool toFSDB = false);

    bool convertToInt();
};

}}

#endif
