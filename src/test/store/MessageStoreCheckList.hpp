#ifndef TEST_STORE_MESSAGE_STORE_CHECK_LIST
#define TEST_STORE_MESSAGE_STORE_CHECK_LIST

namespace smsc {
namespace test {
namespace store {

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class MessageStoreCheckList : public CheckList
{
public:
	MessageStoreCheckList()
		: CheckList("���������� ��������������� ������������ Message Store")
	{
		//storeCorrectSms
		__reg_tc__("storeCorrectSms",
			"���������� sms � ����������� ���������� �����");
		__reg_tc__("storeCorrectSms.origAddrMarginalLength",
			"���������� �������� ����� ������ �����������");
		__reg_tc__("storeCorrectSms.destAddrMarginalLength",
			"���������� �������� ����� ������ ����������");
		__reg_tc__("storeCorrectSms.dealiasedDestAddrMarginalLength",
			"���������� �������� ����� ������������� ������ ����������");
		__reg_tc__("storeCorrectSms.origDescMarginal",
			"���������� �������� imsi � msc ������� � ����������� �����������");
		__reg_tc__("storeCorrectSms.bodyEmpty",
			"������ ���� ���������");
		__reg_tc__("storeCorrectSms.bodyMaxLength",
			"���� short_message � message_payload ������������ �����");
		__reg_tc__("storeCorrectSms.serviceTypeMarginal",
			"���������� �������� ����� serviceType");
		__reg_tc__("storeCorrectSms.rcptMsgIdMaxLength",
			"���� receipted_message_id ������������ �����");
		//storeSimilarSms
		__reg_tc__("storeSimilarSms",
			"���������� sms � ��������� ����������� �������� �� ��� ������������ sms");
		__reg_tc__("storeSimilarSms.diffMsgRef",
			"������ ����������� � ���������� ��������, ���������� msgRef");
		__reg_tc__("storeSimilarSms.diffOrigAddr",
			"MsgRef � ����� ���������� ��������, ���������� ����� �����������");
		__reg_tc__("storeSimilarSms.diffOrigAddr.diffType",
			"���������� ��� ������ �����������");
		__reg_tc__("storeSimilarSms.diffOrigAddr.diffPlan",
			"���������� ���� ������ �����������");
		__reg_tc__("storeSimilarSms.diffOrigAddr.diffValue",
			"���������� ������ ������ �����������");
		__reg_tc__("storeSimilarSms.diffDestAddr",
			"MsgRef � ����� ����������� ��������, ���������� ������ ����������");
		__reg_tc__("storeSimilarSms.diffDestAddr.diffType",
			"���������� ��� ������ ����������");
		__reg_tc__("storeSimilarSms.diffDestAddr.diffPlan",
			"���������� ���� ������ ����������");
		__reg_tc__("storeSimilarSms.diffDestAddr.diffValue",
			"���������� ������ ������ ����������");
		//other
		__reg_tc__("storeDuplicateSms",
			"���������� �������������� sms");
		__reg_tc__("storeRejectDuplicateSms",
			"���������� �������������� sms � �������");
		__reg_tc__("storeReplaceCorrectSms",
			"���������� ����������� sms � ���������� ��� �������������");
		__reg_tc__("storeReplaceSmsInFinalState",
			"���������� sms � �������� ��������� ������������ sms � ��������� ���������");
		__reg_tc__("storeIncorrectSms",
			"���������� ������������� sms (message store �� ��������� �������� ������������ ����� sms)");
		//storeAssertSms
		__reg_tc__("storeAssertSms",
			"���������� ������������� sms � ��������� �� assert");
		__reg_tc__("storeAssertSms.destAddrMarginal",
			"������������ �������� ����� ������ ����������");
		__reg_tc__("storeAssertSms.dealiasedDestAddrMarginal",
			"������������ �������� ����� ������������� ������ ����������");
		__reg_tc__("storeAssertSms.origAddrMarginal",
			"������������ �������� ����� ������ �����������");
		__reg_tc__("storeAssertSms.origDescMscAddrGreaterMaxLength",
			"����� ������ ������ msc � ����������� ����������� ������ ������������");
		__reg_tc__("storeAssertSms.origDescImsiAddrGreaterMaxLength",
			"����� ������ ������ imsi � ����������� ����������� ������ ������������");
		__reg_tc__("storeAssertSms.descMscAddrGreaterMaxLength",
			"����� ������ ������ msc � ����������� ������ ������������");
		__reg_tc__("storeAssertSms.descImsiAddrGreaterMaxLength",
			"����� ������ ������ imsi � ����������� ������ ������������");
		__reg_tc__("storeAssertSms.smGreaterMaxLength",
			"����� ���� short_message ������ ������������");
		__reg_tc__("storeAssertSms.payloadGreaterMaxLength",
			"����� ���� message_payload ������ ������������");
		__reg_tc__("storeAssertSms.serviceTypeGreaterMaxLength",
			"����� ���� service_type ������ ������������");
		__reg_tc__("storeAssertSms.rcptMsgIdGreaterMaxLength",
			"����� ���� receipted_message_id ������ ������������");
		//changeExistentSmsStateEnrouteToEnroute
		__reg_tc__("changeExistentSmsStateEnrouteToEnroute",
			"���������� ������� sms � ��������� ENROUTE (��� ��������� ��������)");
		__reg_tc__("changeExistentSmsStateEnrouteToEnroute.destDescMarginal",
			"���������� �������� ����� imsi � msc ������� � ����������� ����������");
		__reg_tc__("changeExistentSmsStateEnrouteToEnroute.nextTimePast",
			"���� nextTryTime � ������� �������");
		__reg_tc__("changeExistentSmsStateEnrouteToEnroute.lastResultZero",
			"�� ������ �������� lastResult");
		//changeExistentSmsStateEnrouteToFinal
		__reg_tc__("changeExistentSmsStateEnrouteToFinal",
			"������� sms �� ENROUTE � ��������� ���������");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateDeliveredDestDescMarginal",
			"������� � DELIVERED ���������, ���������� �������� imsi � msc ������� � ����������� ����������");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateUndeliverableDestDescMarginal",
			"������� � UNDELIVERABLE ���������, ���������� �������� imsi � msc ������� � ����������� ����������");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateUndeliverableLastResultEmpty",
			"������� � UNDELIVERABLE ���������, �� ������ �������� lastResult");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateExpired",
			"������� � EXPIRED ���������");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateDeleted",
			"������� � DELETED ���������");
		//changeFinalSmsStateToAny
		__reg_tc__("changeFinalSmsStateToAny",
			"������� ��������������� sms ��� sms � ��������� ��������� � ����� ������ ���������");
		__reg_tc__("changeFinalSmsStateToAny.stateEnrote",
			"������� � ENROUTE ���������");
		__reg_tc__("changeFinalSmsStateToAny.stateDelivered",
			"������� � DELIVERED ���������");
		__reg_tc__("changeFinalSmsStateToAny.stateUndeliverable",
			"������� � UNDELIVERABLE ���������");
		__reg_tc__("changeFinalSmsStateToAny.stateExpired",
			"������� � EXPIRED ���������");
		__reg_tc__("changeFinalSmsStateToAny.stateDeleted",
			"������� � DELETED ���������");
		//replaceCorrectSms
		__reg_tc__("replaceCorrectSms",
			"���������� ���������� ������������� sms");
		__reg_tc__("replaceCorrectSms.replaceAll",
			"�������� ��� ��������� ����");
		__reg_tc__("replaceCorrectSms.waitTimeUnchanged",
			"�������� schedule_delivery_time ��� ��������� (���������� ������ 0)");
		__reg_tc__("replaceCorrectSms.validTimeUnchanged",
			"�������� validity_period ��� ��������� (���������� ������ 0)");
		__reg_tc__("replaceCorrectSms.smMarginal",
			"���������� �������� ����� ���� ���������");
		//replaceIncorrectSms
		__reg_tc__("replaceIncorrectSms",
			"������������ ���������� ������������� ��� ���������� ��������������� sms");
		__reg_tc__("replaceIncorrectSms.diffType",
			"������� � ���� ������ �����������");
		__reg_tc__("replaceIncorrectSms.diffPlan",
			"������� � ����� ������ �����������");
		__reg_tc__("replaceIncorrectSms.diffValue",
			"������� � ������ ������ �����������");
		__reg_tc__("replaceIncorrectSms.replaceFinalSms",
			"���������� ��������������� sms ��� sms � ��������� ���������");
		//loadSms
		__reg_tc__("loadSms", "������ sms �� ����");
		__reg_tc__("loadSms.existentSms", "������ ������������� sms");
		__reg_tc__("loadSms.nonExistentSms", "������ ��������������� sms");
		//deleteSms
		__reg_tc__("deleteSms",
			"�������� sms �� ���� (� �������� ������ ��������� ��������� �����������)");
		__reg_tc__("deleteSms.existentSms",
			"�������� ������������� sms");
		__reg_tc__("deleteSms.nonExistentSms",
			"�������� ��������������� sms");
		//checkReadyForRetrySms
		__reg_tc__("checkReadyForRetrySms",
			"��������� sms ��� ��������� ��������");
		__reg_tc__("checkReadyForRetrySms.nextRetryTime",
			"��������� ����� ��������� �������� sms ����������� ���������");
		__reg_tc__("checkReadyForRetrySms.readyForRetrySmsList",
			"������ sms ���������� ��������� �������� ������ � ��������� ����������");
		//not implemented
		/*
		__reg_tc__("deleteExistentWaitingSMByNumber",
			"�������� ������������ sms ��������� �������� �� ������������ �����");
		__reg_tc__("deleteNonExistentWaitingSMByNumber",
			"�������� �������������� sms ��������� �������� �� ������������ �����");
		__reg_tc__("loadExistentWaitingSMByDestinationNumber",
			"�������� ��������� ������ sms ��������� �������� �� ������������ �����");
		__reg_tc__("loadNonExistentWaitingSMByDestinationNumber",
			"�������� ������� ������ sms ��������� �������� �� ������������ �����");
		__reg_tc__("loadExistentSMArchieveByDestinationNumber",
			"�������� ��������� ������ sms ������������ �� ������������ �����");
		__reg_tc__("loadNonExistentSMArchieveByDestinationNumber",
			"�������� ������� ������ sms ������������ �� ������������ �����");
		__reg_tc__("loadExistentSMArchieveByOriginatingNumber",
			"�������� ��������� ������ sms ������������ � ������������� ������");
		__reg_tc__("loadNonExistentSMArchieveByOriginatingNumber",
			"�������� ������� ������ sms ������������ � ������������� ������");
		__reg_tc__("getExistentSMDeliveryFailureStatistics",
			"�������� �������� ���������� ������ ���������� ���������");
		__reg_tc__("getNonExistentSMDeliveryFailureStatistics",
			"�������� ������ ���������� ������ ���������� ���������");
		*/
	}
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_CHECK_LIST */

