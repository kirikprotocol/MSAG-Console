#include "smpp/smpp.h"
#include "util/Exception.hpp"
#include "util/debug.h"

using namespace smsc::smpp;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::util;

void check(SmppHeader* pdu)
{
	//serialize
	__trace__("serialization started");
	int sz = calcSmppPacketLength(pdu);
	char buf[sz];
	SmppStream s1;
	assignStreamWith(&s1, buf, sz, false);
  dump_pdu(pdu);
	if (!fillSmppPdu(&s1, pdu)) throw Exception("Failed to fill smpp packet");
	__trace__("serialization finished");
	//deserialize
	__trace__("deserialization started");
    SmppStream s2;
    assignStreamWith(&s2, buf, sz, true);
    SmppHeader* header = fetchSmppPdu(&s2);
	__trace__("deserialization finished");
    __trace2__("commandId = %x", header->get_commandId());
    disposePdu(header);
}

PduAddress& getAddr(uint8_t ton, uint8_t npi, const char* val)
{
	static PduAddress addr;
	addr.set_typeOfNumber(ton);
	addr.set_numberingPlan(npi);
	addr.set_value(val);
	return addr;
}

void test()
{
	PduMultiSm pdu;
	pdu.get_header().set_commandId(SUBMIT_MULTI);
	pdu.get_header().set_sequenceNumber(1);

    //message
	pdu.get_message().set_serviceType("EOz88");
	pdu.get_message().set_source(getAddr(0, 1, "111"));
	PduDestAddress dests[2];
	dests[0].set_flag(2); //Distribution List Name
	dests[0].set_value("list1");
	dests[1].set_flag(2); //Distribution List Name
	dests[1].set_value("list2");
//	pdu.get_message().set_dests(dests, 2);
	pdu.get_message().set_esmClass(103);
	pdu.get_message().set_protocolId(223);
	pdu.get_message().set_priorityFlag(109);
	pdu.get_message().set_scheduleDeliveryTime("030317135735000+");
	pdu.get_message().set_validityPeriod("030317135827000+");
	pdu.get_message().set_registredDelivery(133);
	pdu.get_message().set_replaceIfPresentFlag(0);
	pdu.get_message().set_dataCoding(0);
	pdu.get_message().set_smDefaultMsgId(102);
	const char msg[] =
	{
		0x31, 0x12, 0x2f, 0x57, 0x3a, 0x4c, 0x4d, 0xe3, 0x09, 0xa6, 0x66, 0xad, 0xbb, 0xf0, 0x3b, 0xd6, 0xf7, 0xc7, 0x6e, 0xac, 0xcf, 0x28, 0x47, 0x22, 0xdd, 0xc0, 0x35, 0x23, 0x4b, 0xcd, 0x38, 0x90, 0xb7, 0x32, 0xfd, 0x40, 0x6e, 0xc1, 0xdc, 0xe5,
		0xfa, 0x65, 0x6e, 0x20, 0x75, 0x3c, 0xfc, 0xa7, 0x9a, 0x3d, 0x40, 0x4d, 0x30, 0x27, 0x36, 0x34, 0x3a, 0x40, 0x38, 0x38, 0x4f, 0x64, 0x58, 0x37, 0x4e, 0x0a, 0x37, 0x34, 0x32, 0x63, 0x2c, 0x5b, 0x69, 0x46, 0x57, 0x24, 0x59, 0x36, 0x32, 0x38,
		0x39, 0x69, 0x2e, 0x70, 0x32, 0x4f, 0x31, 0x53, 0x36, 0x21, 0x20, 0x67, 0x38, 0x5b, 0x70, 0x7e, 0x34, 0x44, 0x6f, 0x0a, 0x33, 0x34, 0x2d, 0x43, 0x75, 0x30, 0x53, 0x4b, 0x78, 0x3c, 0x44, 0x23, 0x47, 0x0a, 0x68, 0x34, 0x0a, 0x32, 0x5c, 0x26,
		0x0a, 0x37, 0x35, 0x21, 0x62, 0x55, 0x61, 0x37, 0x3b, 0x4c, 0x32, 0x4e, 0x21, 0x2e, 0x68, 0x6c, 0x5a, 0x62, 0x29, 0x47, 0x57, 0x39, 0x3a, 0x30, 0x34, 0x79, 0x49, 0x34, 0x57, 0x6c, 0x57, 0x26, 0x0a, 0x6b, 0x36, 0x45, 0x35, 0x75, 0x32, 0x37,
		0x24, 0x35, 0x54, 0x38, 0x69, 0x3d, 0x2d, 0x37, 0x4d, 0x31, 0x6c, 0x5d, 0x36, 0x3e, 0x39, 0x72, 0x5b, 0x32, 0x37, 0x3f, 0x2d, 0x42, 0x29, 0x54, 0x6e, 0x73, 0x36, 0x3c, 0x2f, 0x50, 0x3a, 0x65, 0x32, 0x34, 0x22, 0x2d, 0x38, 0x33
	};
	pdu.get_message().set_shortMessage(msg, 198);

    //optional
    pdu.get_optional().set_destNetworkType(208);
    pdu.get_optional().set_sourceNetworkType(3);
    pdu.get_optional().set_sourceBearerType(119);
    pdu.get_optional().set_qosTimeToLive(67);
    pdu.get_optional().set_payloadType(127);
    pdu.get_optional().set_receiptedMessageId("Xv6vJ99449iCgndA09099");
    pdu.get_optional().set_userMessageReference(1);
    pdu.get_optional().set_sourcePort(23610);
    pdu.get_optional().set_sarSegmentSegnum(29);
    pdu.get_optional().set_setDpf(45);
    pdu.get_optional().set_moreMessagesToSend(70);

    check(reinterpret_cast<SmppHeader*>(&pdu));
}

int main(int argc, char* argv[])
{
	test();
	return 0;
}




