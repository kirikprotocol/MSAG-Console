#include <stdlib.h>
#include <util/recoder/recode_dll.h>
#include "Strategy_SendPositiveTransliteratedMessage.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

void
Strategy_SendPositiveTransliteratedMessage::handle()
{
  smsc::inman::interaction::SPckUSSResult* resultPacket = new smsc::inman::interaction::SPckUSSResult();

  int rnd = rand()%100;
  if ( rnd >= 0 && rnd < 25 ) {
    char transliteratedSuccessfulResultStr[128];
    snprintf(transliteratedSuccessfulResultStr, sizeof(transliteratedSuccessfulResultStr), "Vash balans %d rub.", rnd);
    resultPacket->Cmd().setUSSData(transliteratedSuccessfulResultStr,
                                   (unsigned)strlen(transliteratedSuccessfulResultStr));
  } else if ( rnd >= 25 && rnd < 50 ) {
    char transliteratedSuccessfulResultStr[128];
    snprintf(transliteratedSuccessfulResultStr, sizeof(transliteratedSuccessfulResultStr), "Vash balans -%d rub.", rnd);
    resultPacket->Cmd().setUSSData(transliteratedSuccessfulResultStr,
                                   (unsigned)strlen(transliteratedSuccessfulResultStr));
  } else if ( rnd >= 50 && rnd < 75 ) {
    char vash_balans_str[128];
    snprintf(vash_balans_str, sizeof(vash_balans_str), "Ваш баланс %d руб.", rnd);

    resultPacket->Cmd().setUCS2USSData(prepareStringInUCS(vash_balans_str));
  } else {
    char vash_dolg_str[128];
    snprintf(vash_dolg_str, sizeof(vash_dolg_str), "Ваш баланс -%d руб.", rnd);
 
    resultPacket->Cmd().setUCS2USSData(prepareStringInUCS(vash_dolg_str));
  }

  resultPacket->Cmd().setStatus(smsc::inman::interaction::USS2CMD::STATUS_USS_REQUEST_OK);
  resultPacket->Cmd().setMSISDNadr(_requestObject.getMSISDNadr());
  resultPacket->setDialogId(_dialogId);

  _responseWriter->scheduleResponse(resultPacket, _conn);
}

std::vector<uint8_t>
Strategy_SendPositiveTransliteratedMessage::prepareStringInUCS(const char* textString)
{
  union {
    short ucs2ResultStr[128];
    uint8_t ucs2OctetsString[128*2];
  } buf4resultStr;
  int resultSizeInOctets = ConvertMultibyteToUCS2(textString, strlen(textString), buf4resultStr.ucs2ResultStr, sizeof(buf4resultStr.ucs2ResultStr), CONV_ENCODING_KOI8R);

  return std::vector<uint8_t>(buf4resultStr.ucs2OctetsString,buf4resultStr.ucs2OctetsString + resultSizeInOctets);
}

}}}
