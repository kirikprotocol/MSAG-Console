/* $Id$ */

#ifndef SCAG_PERS_STORE_PROPERTY
#define SCAG_PERS_STORE_PROPERTY

#include <list>
#include <string>

#include "SerialBuffer.h"

namespace scag{ namespace pers{

using namespace std;
using namespace smsc::core::buffers;

enum TimePolicy{
	INFINIT = 1,
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
	string name;
	wstring s_val;
	uint32_t i_val;
	time_t d_val;
	bool b_val;

	TimePolicy time_policy;
	time_t final_date;
	uint32_t life_time;

	void copy(const Property& cp);
	static void StringFromBuf(SerialBuffer& buf, string &str);
	static void StringFromBuf(SerialBuffer& buf, wstring &str);
	void ReadAccess();
	void WriteAccess();

public:

	Property() : type(INT), i_val(0), time_policy(INFINIT){};
	Property(const Property& cp);
	Property& operator=(const Property& cp);

	Property(const char *nm, uint32_t i, TimePolicy policy, time_t fd, uint32_t lt)
	{
		setInt(nm, i, policy, fd, lt);
	};

	Property(const char *nm, const wchar_t* str, TimePolicy policy, time_t fd, uint32_t lt)
	{
		setString(nm, str, policy, fd, lt);
	};


	const string& getName() { return name; };
	void setName(const string& nm) { name = nm; };
	uint32_t getIntValue() { ReadAccess(); return i_val; };
	bool getBoolValue() { ReadAccess(); return b_val; };
	time_t getDateValue() { ReadAccess(); return d_val; };
	const wstring& getStringValue() { ReadAccess(); return s_val; };
	void setIntValue(uint32_t i) { WriteAccess(); i_val = i; type = INT; };
	void setBoolValue(bool b) { WriteAccess(); b_val = b; type = BOOL; };
	void setDateValue(time_t d) { WriteAccess(); d_val = d; type = DATE; };
	void setStringValue(const wchar_t* s) { WriteAccess(); s_val = s; type = STRING; };
	uint8_t getType() { return type; };
	void setTimePolicy(TimePolicy policy, time_t fd, uint32_t lt);
	bool isExpired();
	string toString() const;

	void setInt(const char *nm, uint32_t i, TimePolicy policy, time_t fd, uint32_t lt);
	void setBool(const char *nm, bool b, TimePolicy policy, time_t fd, uint32_t lt);
	void setString(const char *nm, const wchar_t* str, TimePolicy policy, time_t fd, uint32_t lt);
	void setDate(const char *nm, time_t t, TimePolicy policy, time_t fd, uint32_t lt);

	void Serialize(SerialBuffer& buf);
	void Deserialize(SerialBuffer& buf);
};

}}

#endif
