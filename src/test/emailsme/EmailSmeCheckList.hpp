#ifndef TEST_EMAIL_SME_EMAIL_SME_CHECK_LIST
#define TEST_EMAIL_SME_EMAIL_SME_CHECK_LIST

#include "test/sme/SystemSmeCheckList.hpp"

namespace smsc {
namespace test {
namespace emailsme {

using smsc::test::sme::SystemSmeCheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
#define __hide_tc__(id) \
	hideTc(id)

class EmailSmeCheckList : public SystemSmeCheckList
{

void aaa()
{
	__reg_tc__("sendMailCorrect.existentAddress",
		"Отправка сообщения на существующий email адрес");
	__reg_tc__("sendMailCorrect.nonExistentAddress",
		"Отправка сообщения на несуществующий email адрес");
	__reg_tc__("sendMailCorrect.addressList",
		"Отправка сообщения на список email адресов");
}

public:
	EmailSmeCheckList()
		: SystemSmeCheckList("Результаты функционального тестирования email sme", "email.chk")
	{
		__hide_tc__("bindIncorrectSme");
		__hide_tc__("submitSm.correct");
		__hide_tc__("submitSm.incorrect");
		__hide_tc__("submitSm.assert");
		__hide_tc__("replaceSm");
		__hide_tc__("sendDeliverySmResp.sendRetry");
		__hide_tc__("sendDeliverySmResp.sendError");
		__hide_tc__("processReplaceSmResp");
		__hide_tc__("processDeliverySm.normalSms");
		//__hide_tc__("processDeliverySm.deliveryReceipt");
		__hide_tc__("processDeliverySm.intermediateNotification");
		__hide_tc__("notImplemented");
		__hide_tc__("updateProfile");
		__hide_tc__("processUpdateProfile");

		//submitDbSmeCmd.correct
		__reg_tc__("submitDbSmeCmd", "Отправка sms на db sme");
		__reg_tc__("submitDbSmeCmd.cmdTextDefault",
			"Текст команды в дефолтной кодировке SC");
		__reg_tc__("submitDbSmeCmd.cmdTextUcs2",
			"Текст команды в кодировке UCS2");
		__reg_tc__("submitDbSmeCmd.jobNameMixedCase",
			"Имя задачи в смешанном регистре (заглавные и прописные буквы)");
		__reg_tc__("submitDbSmeCmd.dateMixedCase",
			"Дата в смешанном регистре (заглавные и прописные буквы в имени месяца или AM/PM индикаторе)");
		__reg_tc__("submitDbSmeCmd.correct", "Отправка правильной команды db sme");
		correctJobTc();
		correctInputTc();
		incorrectInputTc();
		//result
		processDbSmeRes();
	}
};

}
}
}

#endif /* TEST_EMAIL_SME_EMAIL_SME_CHECK_LIST */

