/* $Id$ */

#include "Property.h"

namespace scag{ namespace pers{

void Property::copy(const Property& cp)
{
	name = cp.name;
	type = cp.type;
	time_policy = cp.time_policy;
	final_date = cp.final_date;
	life_time = cp.life_time;

	switch(type) {
		case INT:
			i_val = cp.i_val;
			break;
		case STRING:
			s_val = cp.s_val;
			break;
		case BOOL:
			b_val = cp.b_val;
			break;
		case DATE:
			d_val = cp.d_val;
			break;
	}
}

string Property::toString() const
{
	char buf[32];
	string str;

	str = name;
	switch(type)
	{
		case INT:
			sprintf(buf, " INT: %d", i_val);
			str += buf;
			break;
		case STRING:
			str += " STRING: ";// + s_val;
			break;
		case BOOL:
			str += " BOOL: ";
			str += (b_val ? "true" : "false");
			break;
		case DATE:
			strftime(buf, 32, "%Y/%m/%d %H:%M:%S", gmtime(&d_val));
			str += " DATE: ";
			str += buf;
			break;
	}

	str += " TIME_POLICY: ";
	switch(time_policy)
	{
		case FIXED:
			str += "FIXED";
			break;
		case INFINIT:
			str += "INFINIT";
			break;
		case R_ACCESS:
			str += "R_ACCESS";
			break;
		case W_ACCESS:
			str += "W_ACCESS";
			break;
		case ACCESS:
			str += "ACCESS";
			break;
	}
	strftime(buf, 32, "%Y/%m/%d %H:%M:%S", gmtime(&final_date));
	str += " FINAL_DATE: ";
	str += buf;
	sprintf(buf, "%d", life_time);
	str += " LIFE_TIME: ";
	str += buf;
	return str;
}

Property::Property(const Property& cp)
{
	copy(cp);
}

Property& Property::operator=(const Property& cp)
{
	copy(cp);
	return *this;
}

void Property::setTimePolicy(TimePolicy policy, time_t fd, uint32_t lt)
{
	time_policy = policy;
	life_time = lt;
	final_date = (policy == FIXED && fd != -1) ? fd : final_date = time(NULL) + lt;
}

void Property::ReadAccess()
{
	if(time_policy == R_ACCESS || time_policy == ACCESS)
		final_date = time(NULL) + life_time;
}

void Property::WriteAccess()
{
	if(time_policy == W_ACCESS || time_policy == ACCESS)
		final_date = time(NULL) + life_time;
}

bool Property::isExpired()
{
	return time_policy != INFINIT && final_date <= time(NULL);
}

void Property::setInt(const char *nm, uint32_t i, TimePolicy policy, time_t fd, uint32_t lt)
{
	name = nm;
	setIntValue(i);
	setTimePolicy(policy, fd, lt);
}

void Property::setBool(const char *nm, bool b, TimePolicy policy, time_t fd, uint32_t lt)
{
	name = nm;
	setBoolValue(b);
	setTimePolicy(policy, fd, lt);
}

void Property::setString(const char *nm, const wchar_t* str, TimePolicy policy, time_t fd, uint32_t lt)
{
	name = nm;
	setStringValue(str);
	setTimePolicy(policy, fd, lt);
}

void Property::setDate(const char *nm, time_t t, TimePolicy policy, time_t fd, uint32_t lt)
{
	name = nm;
	setDateValue(t);
	setTimePolicy(policy, fd, lt);
}

void Property::Serialize(SerialBuffer& buf)
{
	uint16_t len = name.length();

	uint8_t t = type;
	buf.Append((char*)&t, sizeof(uint8_t));
	t = time_policy;
	buf.Append((char*)&t, sizeof(uint8_t));
	buf.Append((char*)&final_date, sizeof(time_t));
	buf.Append((char*)&life_time, sizeof(uint32_t));
	buf.Append((char*)&len, sizeof(len));
	buf.Append(name.c_str(), len);

	switch(type) {
		case INT:
			buf.Append((char*)&i_val, sizeof(uint32_t));
			break;

		case STRING:
			len = s_val.length();
			buf.Append((char*)&len, sizeof(len));
			buf.Append((char*)s_val.c_str(), sizeof(wchar_t) * len);
			break;

		case BOOL:
			buf.Append((char*)&b_val, sizeof(bool));
			break;

		case DATE:
			buf.Append((char*)&d_val, sizeof(time_t));
			break;
	}
}

void Property::Deserialize(SerialBuffer& buf)
{
	string str;
	uint8_t len;
	uint8_t t;

	buf.Read((char*)&t, sizeof(uint8_t));
	type = (PropertyType)t;
	buf.Read((char*)&t, sizeof(uint8_t));
	time_policy = (TimePolicy)t;
	buf.Read((char*)&final_date, sizeof(time_t));
	buf.Read((char*)&life_time, sizeof(uint32_t));

	buf.ReadString(name);

	switch(type) {
		case INT:
			buf.Read((char*)&i_val, sizeof(uint32_t));
			break;

		case STRING:
			buf.ReadString(s_val);
			break;

		case BOOL:
			buf.Read((char*)&b_val, sizeof(bool));
			break;

		case DATE:
			buf.Read((char*)&d_val, sizeof(time_t));
			break;
	}
}

}}
