#ifndef TEST_SME_SMSC_SME_MESSAGES
#define TEST_SME_SMSC_SME_MESSAGES

#include "test/sms/SmsUtil.hpp"
#include "test/util/DateFormatter.hpp"
#include "test/util/TextUtil.hpp"
#include "system/status.h"
#include "util/debug.h"
#include <string>
#include <sstream>

namespace smsc {
namespace test {
namespace sme {

using std::string;
using std::ostringstream;
using smsc::test::sms::SmsUtil;
using smsc::test::util::DateFormatter;
using smsc::test::util::convert;
using smsc::system::Status;
using namespace smsc::smpp::DataCoding;

struct SmscSmeMessage
{
	static pair<string, uint8_t>& trim(pair<string, uint8_t>& p)
	{
		switch (p.second)
		{
			case DEFAULT:
			case SMSC7BIT:
				if (p.first.length() > 160)
				{
					p.first.replace(157, p.first.length() - 157, "...");
				}
				return p;
			case UCS2:
				if (p.first.length() > 70)
				{
					p.first.replace(67, p.first.length() - 67, "...");
				}
				return p;
			default:
				__unreachable__("Invalid data coding");
		}
	}
	static const char* getErrDescEn(int reason)
	{
		switch (reason & 0xffff)
		{
			case Status::INVMSGLEN:
				return "Message Length is invalid";
			case Status::INVCMDLEN:
				return "Command Length is invalid";
			case Status::INVCMDID:
				return "Invalid Command ID";
			case Status::INVBNDSTS:
				return "Incorrect BIND Status for given command";
			case Status::ALYBND:
				return "ESME Already in Bound State";
			case Status::INVPRTFLG:
				return "Invalid Priority Flag";
			case Status::INVREGDLVFLG:
				return "Invalid Registered Delivery Flag";
			case Status::SYSERR:
				return "System Error";
			case Status::INVSRCADR:
				return "Invalid Source Address";
			case Status::INVDSTADR:
				return "Invalid Dest Addr";
			case Status::INVMSGID:
				return "Message ID is invalid";
			case Status::BINDFAIL:
				return "Bind Failed";
			case Status::INVPASWD:
				return "Invalid Password";
			case Status::INVSYSID:
				return "Invalid System ID";
			case Status::CANCELFAIL:
				return "Cancel SM Failed";
			case Status::REPLACEFAIL:
				return "Replace SM Failed";
			case Status::MSGQFUL:
				return "Message Queue Full";
			case Status::INVSERTYP:
				return "Invalid Service Type";
			case Status::INVNUMDESTS:
				return "Invalid number of destinations";
			case Status::INVDLNAME:
				return "Invalid Distribution List name";
			case Status::INVDESTFLAG:
				return "Destination flag is invalid";
			case Status::INVSUBREP:
				return "Invalid 'submit with replace' request";
			case Status::INVESMCLASS:
				return "Invalid esm_class field data";
			case Status::CNTSUBDL:
				return "Cannot Submit to Distribution List";
			case Status::SUBMITFAIL:
				return "submit_sm or submit_multi failed";
			case Status::INVSRCTON:
				return "Invalid Source address TON";
			case Status::INVSRCNPI:
				return "Invalid Source address NPI";
			case Status::INVDSTTON:
				return "Invalid Destination address TON";
			case Status::INVDSTNPI:
				return "Invalid Destination address NPI";
			case Status::INVSYSTYP:
				return "Invalid system_type field";
			case Status::INVREPFLAG:
				return "Invalid replace_if_present flag";
			case Status::INVNUMMSGS:
				return "Invalid number of messages";
			case Status::THROTTLED:
				return "Throttling error";
			case Status::INVSCHED:
				return "Invalid Scheduled Delivery Time";
			case Status::INVEXPIRY:
				return "Invalid message validity period";
			case Status::INVDFTMSGID:
				return "Predefined Message Invalid or Not Found";
			case Status::RX_T_APPN:
				return "ESME Receiver Temporary App Error Code";
			case Status::RX_P_APPN:
				return "ESME Receiver Permanent App Error Code";
			case Status::RX_R_APPN:
				return "ESME Receiver Reject Message Error Code";
			case Status::QUERYFAIL:
				return "query_sm request failed";
			case Status::INVOPTPARSTREAM:
				return "Error in the optional part of the PDU Body";
			case Status::OPTPARNOTALLWD:
				return "Optional Parameter not allowed";
			case Status::INVPARLEN:
				return "Invalid Parameter Length";
			case Status::MISSINGOPTPARAM:
				return "Expected Optional Parameter missing";
			case Status::INVOPTPARAMVAL:
				return "Invalid Optional Parameter Value";
			case Status::DELIVERYFAILURE:
				return "Delivery Failure";
			case Status::UNKNOWNERR:
				return "Unknown Error";
			case Status::INVDCS:
				return "Invalid Data Coding Scheme";

			case Status::NOROUTE:
				return "No route";
			case Status::EXPIRED:
				return "Message expired";
			case Status::DELIVERYTIMEDOUT:
				return "Delivery attempt timed out";
			case Status::SMENOTCONNECTED:
				return "Sme is not connected";
			case Status::BLOCKEDMSC:
				return "Msc is blocked";

			case Status::UNKSUBSCRIBER:
				return "Unknown subscriber";
			case Status::UNDEFSUBSCRIBER:
				return "Unidentified subscriber";
			case Status::ILLEGALSUBSCRIBER:
				return "Illegal subscriber";
			case Status::INVEQUIPMENT:
				return "Illegal Equipment";
			case Status::CALLBARRED:
				return "Call Barred";
			case Status::FACILITYNOTSUPP:
				return "Facility Not Supported";
			case Status::ABSENTSUBSCR:
				return "Absent Subscriber";
			case Status::SUBSCRBUSYMT:
				return "Subscriber Busy for MT SMS.";
			case Status::SMDELIFERYFAILURE:
				return "SM Delivery Failure";
			case Status::SYSFAILURE:
				return "System Failure";
			case Status::DATAMISSING:
				return "Data Missing";
			case Status::UNEXPDATA:
				return "Unexpected Data value";
			default:
				return NULL;
		}
	}
	static const char* getErrDescRu(int reason)
	{
		switch (reason & 0xffff)
		{
			case Status::INVBNDSTS:
				return "Неправильный тип соединения sme";
			case Status::MSGQFUL:
				return "Переполнение очереди";
			case Status::RX_T_APPN:
				return "Временная ошибка";
			case Status::EXPIRED:
				return "Сообщение протухло";
			case Status::DELIVERYTIMEDOUT:
				return "Таймаут при доставке сообщения";
			default:
				return NULL;
		}
	}
	static const string getErrDesc(const Profile& profile, int reason)
	{
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			const char* errDesc = getErrDescEn(reason);
			if (errDesc)
			{
				return errDesc;
			}
			ostringstream s;
			s << reason;
			return s.str();
		}
		if (profile.locale == "ru_ru")
		{
			const char* errDesc = getErrDescRu(reason);
			if (errDesc)
			{
				return errDesc;
			}
			errDesc = getErrDescEn(reason);
			if (errDesc)
			{
				return errDesc;
			}
			ostringstream s;
			s << reason;
			return s.str();
		}
		__unreachable__("Invalid locale");
	}
};

//Сообщения всегда умещаются на 1 sms
//Латиница для en, кирилица для ru
struct SmscSmeDeliveredReceipt : SmscSmeMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const Address& destAlias, time_t deliveryTime)
	{
//*Your message sent to .0.0.12345678901234567890 was successfully delivered on 01 January 2003, 12:00:00
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			static const DateFormatter df("dd MMMM yyyy, HH:mm:ss");
			ostringstream s;
			s << "*Your message sent to ";
			s << SmsUtil::configString(destAlias);
			s << " was successfully delivered on ";
			s << df.format(deliveryTime);
			pair<string, uint8_t> p = convert(s.str(), profile.codepage);
			return trim(p);
		}
//*Сообщение на .0.0.12345678901234567890 доставлено 01-01-03 12:00:00
		if (profile.locale == "ru_ru")
		{
			static const DateFormatter df("dd-MM-yy HH:mm:ss");
			ostringstream s;
			s << "*Сообщение на ";
			s << SmsUtil::configString(destAlias);
			s << " доставлено ";
			s << df.format(deliveryTime);
			pair<string, uint8_t> p = convert(s.str(), profile.codepage);
			return trim(p);
		}
		__unreachable__("Invalid locale");
	}
};

//Сообщения заведомо не умещаются на 1 sms
//Латиница для en, кирилица для ru
struct SmscSmeFailedReceipt : SmscSmeMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const Address& destAlias, time_t sendTime, int status)
	{
//*Message to +123 sent on 01 January 2003, 12:00:00 failed: temporary error. Resend the message to +123 once again and maybe you will have luck next time. Sincerely, smsc sme.
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			static const DateFormatter df("dd MMMM yyyy, HH:mm:ss");
			ostringstream s;
			s << "*Message to ";
			s << SmsUtil::configString(destAlias);
			s << " sent on ";
			s << df.format(sendTime);
			s << " failed: ";
			s << getErrDesc(profile, status);
			s << ". Resend the message to ";
			s << SmsUtil::configString(destAlias);
			s << " once again and maybe you will have luck next time. Sincerely, smsc sme.";
			pair<string, uint8_t> p = convert(s.str(), profile.codepage);
			return trim(p);
		}
//*Сообщение на +123 от 01-01-03 12:00:00 обломалось: временная ошибка. Пошлите сообщение на +123 еще раз и возможно вам повезет на этот раз. С уважением, smsc sme.
		if (profile.locale == "ru_ru")
		{
			static const DateFormatter df("dd-MM-yy HH:mm:ss");
			ostringstream s;
			s << "*Сообщение на ";
			s << SmsUtil::configString(destAlias);
			s << " от ";
			s << df.format(sendTime);
			s << " обломалось: ";
			s << getErrDesc(profile, status);
			s << ". Пошлите сообщение на ";
			s << SmsUtil::configString(destAlias);
			s << " еще раз и возможно вам повезет на этот раз. С уважением, smsc sme.";
			pair<string, uint8_t> p = convert(s.str(), profile.codepage);
			return trim(p);
		}
		__unreachable__("Invalid locale");
	}
};

//Сообщения заведомо умещаются на 1 sms
//Наследование из базовой локали
struct SmscSmeDeletedReceipt : SmscSmeMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const Address& destAlias, time_t sendTime, int status)
	{
//*Сообщение на .0.0.12345678901234567890 от 01-01-03 12:00:00 удалено
		static const DateFormatter df("dd-MM-yy HH:mm:ss");
		ostringstream s;
		s << "*Сообщение на ";
		s << SmsUtil::configString(destAlias);
		s << " от ";
		s << df.format(sendTime);
		s << " удалено";
		pair<string, uint8_t> p = convert(s.str(), profile.codepage);
		return trim(p);
	}
};

//Пустое сообщение
struct SmscSmeNotification : SmscSmeMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const Address& destAlias, time_t sendTime, int status)
	{
//$Сообщение на +123 от 01-01-03 12:00:00 обломалось: временная ошибка
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			static const DateFormatter df("dd-MM-yy HH:mm:ss");
			ostringstream s;
			s << "$Сообщение на ";
			s << SmsUtil::configString(destAlias);
			s << " от ";
			s << df.format(sendTime);
			s << " обломалось: ";
			s << getErrDesc(profile, status);
			pair<string, uint8_t> p = convert(s.str(), profile.codepage);
			return trim(p);
		}
		if (profile.locale == "ru_ru")
		{
			return convert("$", profile.codepage);
		}
		__unreachable__("Invalid locale");
	}
};

}
}
}

#endif /* TEST_SME_SMSC_SME_MESSAGES */
