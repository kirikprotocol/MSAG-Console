#include "system/common/TimeZoneMan.hpp"

/*struct SMS_DELIVERY_FORMAT_HEADER{
  union{
    struct{
      unsigned char reply_path:1;
      unsigned char udhi:1;
      unsigned char srri:1;
      unsigned char reserved:2;
      unsigned char mms:1;
      unsigned char mg_type_ind:2;
    }s;
    unsigned char _val_01;
  }uu;
};
*/

static uint8_t uint8tohalfoct(uint8_t val)
{
  uint8_t high=val/10;
  uint8_t low=val%10;
  return (low<<4)|high;
}

void fillPduTime(MAP_TIMESTAMP* pdu_tm,struct tm* tms,int atz)
{
  pdu_tm->year = uint8tohalfoct(tms->tm_year%100);
  pdu_tm->mon = uint8tohalfoct(tms->tm_mon+1);
  pdu_tm->day = uint8tohalfoct(tms->tm_mday);
  pdu_tm->hour = uint8tohalfoct(tms->tm_hour);
  pdu_tm->min  =  uint8tohalfoct(tms->tm_min);
  pdu_tm->sec = uint8tohalfoct(tms->tm_sec);
  if ( tms->tm_isdst ) atz-=3600;
  atz = -atz/900;
  pdu_tm->tz=(atz%10)<<4;
  if(atz<0)
  {
    atz=-atz;
    pdu_tm->tz|=0x8|(atz/10);
  }else
  {
    pdu_tm->tz|=atz/10;
  }
}

ET96MAP_SM_RP_UI_T* mkDeliverPDU(SMS* sms,ET96MAP_SM_RP_UI_T* pdu,bool mms=false)
{
#if defined USE_MAP
  memset(pdu,0,sizeof(ET96MAP_SM_RP_UI_T));
  int esm=sms->getIntProperty(Tag::SMPP_ESM_CLASS);
  int isrcpt=(esm&0x3c)==4 || (esm&0x3c)==0x20;
  //SMS_DELIVERY_FORMAT_HEADER* header = (SMS_DELIVERY_FORMAT_HEADER*)pdu->signalInfo;

  pdu->signalInfo[0]=0;
  pdu->signalInfo[0]|=(isrcpt?2:0);
  pdu->signalInfo[0]|=(mms?0:1)<<2;
  pdu->signalInfo[0]|=((esm&0x40)?1:0)<<6;
  pdu->signalInfo[0]|=((esm&0x80)?1:0)<<7;

  /*header->uu.s.mg_type_ind = isrcpt?2:0;
  header->uu.s.mms = !mms;
  header->uu.s.reply_path = (esm&0x80)?1:0;;
  header->uu.s.srri = 0;
  header->uu.s.udhi = (esm&0x40)?1:0;
  */
  unsigned char *pdu_ptr = pdu->signalInfo+1;
  MAP_SMS_ADDRESS* oa;
  Address addr;
  if(isrcpt)
  {

    *pdu_ptr++=sms->getMessageReference();
    oa = (MAP_SMS_ADDRESS*)pdu_ptr;
    if(!sms->getStrProperty(Tag::SMSC_RECIPIENTADDRESS).length())
    {
      throw MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_PERM,smsc::system::Status::INVESMCLASS),"receipt without recipient address");
    }
    addr=sms->getStrProperty(Tag::SMSC_RECIPIENTADDRESS).c_str();
  }else
  {
    oa = (MAP_SMS_ADDRESS*)(pdu->signalInfo+1);
    addr=sms->getOriginatingAddress();
  }
  int ton=addr.getTypeOfNumber()&0x07;
  int npi=addr.getNumberingPlan();
  oa->tonnpi = 0x80|(ton<<4)|(npi&0x0f);
  unsigned oa_length = (oa->len+1)/2;
  if ( ton == 5 )
  {
    if (addr.getLength()>11) throw runtime_error(":MAP: invalid address length");
    unsigned tmpX = 0;
    unsigned _7bit_text_len = ConvertText27bit((const unsigned char*)addr.value,addr.length,oa->val,&tmpX,0,10);
    oa->len = _7bit_text_len*2;
    oa_length = _7bit_text_len;
    if( _7bit_text_len*8-addr.length*7 == 7 ) {
      unsigned char c = oa->val[_7bit_text_len-1];
      oa->val[_7bit_text_len-1] = (0x0d<<1)|(oa->val[_7bit_text_len-1]&0x01);
    }
  }
  else
  {
    oa->len = addr.getLength();
    oa_length = (oa->len+1)/2;
    char* sval = addr.value;
    for ( int i=0; i<oa->len; ++i ){
      int bi = i/2;
      if( i%2 == 1 ){
        oa->val[bi] |= ((sval[i]-'0')<<4); // fill high octet
      }else{
        oa->val[bi] = (sval[i]-'0')&0x0F; // fill low octet
      }
    }
    if( oa->len%2 != 0 ) oa->val[oa_length-1] |= 0xF0;
  }
  pdu_ptr+=2+oa_length;
  if(!isrcpt)
  {
    *pdu_ptr++ = (unsigned char)sms->getIntProperty(Tag::SMPP_PROTOCOL_ID);
  }
  unsigned encoding = sms->getIntProperty(Tag::SMPP_DATA_CODING);
  unsigned char datacoding;
  if ( encoding != 0 &&
       encoding != 0x08 &&
       encoding != 0x03 &&
       encoding != 0x04 &&
       encoding != 0xf0) {
    __map_warn2__("MAP::mkDeliverPDU: unsuppprted encoding 0x%x",encoding);
    throw runtime_error("unsupported encoding");
  }
  else // make coding scheme
  {
    if ( sms->getIntProperty(Tag::SMSC_FORCE_DC) ) {
      datacoding = sms->getIntProperty(Tag::SMSC_ORIGINAL_DC);
    }
    else if (sms->hasIntProperty(Tag::SMPP_MS_VALIDITY)) {
      unsigned ms_validity = sms->getIntProperty(Tag::SMPP_MS_VALIDITY);
      if (sms->hasIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES)) {
        unsigned ms_wait_facilities = sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES);
        if ( ms_validity != 0x03 && ms_validity != 0 ) throw runtime_error("Invalid ms_validity value for MWI control");
        if ( encoding == MAP_OCTET7BIT_ENCODING || encoding == MAP_LATIN1_ENCODING ) {
          datacoding = 0xc0
            |(ms_validity==0x03?0:0x10)
            |(ms_wait_facilities&0x03)
            |((ms_wait_facilities&0x80)>>4);
        }
        else if ( encoding == MAP_UCS2_ENCODING ) {
          datacoding = 0xe0
            |(ms_wait_facilities&0x03)
            |((ms_wait_facilities&0x80)>>4);
        }
        else throw runtime_error("Invalid encoding for MWI control");
      }
      else { // 01xxxxxx
        unsigned code = encoding;
        if ( code == MAP_LATIN1_ENCODING ) code = MAP_OCTET7BIT_ENCODING;
        datacoding = 0x40 | (code & 0x0c);
        if ( sms->hasIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT) )
          datacoding |= (sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)-1)&0x3;
      }
    }
    else
    {
      unsigned code = encoding;
      if ( code == MAP_LATIN1_ENCODING ) code = MAP_OCTET7BIT_ENCODING;
      datacoding = code&0x0c;
      if ( sms->hasIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT) ) {
        if( code == MAP_UCS2_ENCODING ) datacoding |= 0x10;
        else datacoding |= 0xf0;
        datacoding |= (sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)-1)&0x3;
      }
    }
#if 0
    if ( sms->getIntProperty(Tag::SMSC_FORCE_DC) ) {
      datacoding = sms->getIntProperty(Tag::SMSC_ORIGINAL_DC);
    }
    else if ( sms->hasIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT) ){
      // coding scheme 1111xxxx
      datacoding = 0xf0;
      if ( encoding == MAP_UCS2_ENCODING){
        throw runtime_error("Coding group 1111xxxx incompatible with encoding UCS2");
      }
      if ( encoding == MAP_OCTET7BIT_ENCODING || encoding == MAP_LATIN1_ENCODING || encoding == MAP_SMSC7BIT_ENCODING)
        ;//datacoding |=  nothing
      else // 8bit
        datacoding |= (1<<2);
      datacoding |= (sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)-1)&0x3;
    }
    else
    {
      if ( sms->hasIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES) ){
        if ( encoding == MAP_UCS2_ENCODING ){
          datacoding = 0xe0;
          unsigned _val = sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES);
          datacoding |= _val&0x3;
          if ( _val&0x80 )datacoding |= 0x8;
        }
        else
        {
          if ( !sms->hasIntProperty(Tag::SMPP_MS_VALIDITY) ){
            throw runtime_error("MAP::mkDeliveryPDU: Opss, has no ms_validity");
          }
          unsigned ms_validity = sms->getIntProperty(Tag::SMPP_MS_VALIDITY);
          if ( (ms_validity & 0x3) == 0x3 ){
            datacoding = 0xc0;
          }else if ( (ms_validity & 0x3) == 0 ){
            datacoding = 0xd0;
          }else{
            __map_warn2__("mkDeliveryPDU: Opss, ms_validity = 0x%x but must be 0x0 or 0x3",
                       ms_validity);
            throw runtime_error("bad ms_validity value");
          }
          unsigned _val = sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES);
          datacoding |= _val&0x3;
          if ( _val&0x80 )datacoding |= 0x8;
        }
      }
      else
      {
        /*if ( !sms->hasIntProperty(Tag::MS_VALIDITY) ){
          __map_trace2__("mkDeliveryPDU: Opss, has no ms_validity");
          throw runtime_error("MAP::mkDeliveryPDU: Opss, has no ms_validity");
        }*/
        unsigned ms_validity = 0;
        if ( sms->hasIntProperty(Tag::SMPP_MS_VALIDITY) )
          ms_validity = sms->getIntProperty(Tag::SMPP_MS_VALIDITY);
        if ( (ms_validity & 0x3) == 0x3 ){
          datacoding = 0x40;
        }else if ( (ms_validity & 0x3) == 0 ){
          datacoding = 0x00;
        }else{
          __map_warn2__("mkDeliveryPDU: Opss, ms_validity = 0x%x but must be 0x0 or 0x3",
                     ms_validity);
          throw runtime_error("bad ms_validity value");
        }
        if ( encoding == MAP_UCS2_ENCODING ){
           datacoding |= 0x8;
        }
        //if ( encoding == 0 && encoding == 0x3 ) nothing
        if ( encoding == MAP_8BIT_ENCODING ){
           datacoding |= 0x4;
        }
      }
    }
#endif
//  __map_trace2__("MAP::mkDeliveryPDU: encoding = 0x%x",encoding);
//    __map_trace2__("mkDeliveryPDU: map DCS = 0x%x",(unsigned)datacoding);
    if(!isrcpt)
    {
      if ( sms->getIntProperty(Tag::SMSC_FORCE_DC) ) {
        *pdu_ptr++ = sms->getIntProperty(Tag::SMSC_ORIGINAL_DC);
      }else
        *pdu_ptr++ = datacoding;
    }
  }
  {

    time_t t = sms->getSubmitTime();
    if( isrcpt && sms->hasIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME) ) {
      t = sms->getIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME);
    }
//    time(&t);
    int abonentsTz=smsc::system::common::TimeZoneManager::getInstance().getTimeZone(sms->getOriginatingAddress());
    abonentsTz=-abonentsTz;
    struct tm tms;
    if(timezone!=abonentsTz)
    {
      t+=(timezone-abonentsTz);
    }
    localtime_r(&t,&tms);
    fillPduTime((MAP_TIMESTAMP*)pdu_ptr,&tms,abonentsTz);
    pdu_ptr+=sizeof(MAP_TIMESTAMP);
    if(isrcpt)
    {
      t=sms->getIntProperty(Tag::SMSC_DISCHARGE_TIME);
      if(timezone!=abonentsTz)
      {
        t+=(timezone-abonentsTz);
      }
      localtime_r(&t,&tms);
      fillPduTime((MAP_TIMESTAMP*)pdu_ptr,&tms,abonentsTz);
      pdu_ptr+=sizeof(MAP_TIMESTAMP);
      //!!!TODO!!! expired
      switch(sms->getIntProperty(Tag::SMPP_MSG_STATE))
      {
        case SmppMessageState::DELIVERED: *pdu_ptr++=0;break; //ok
        case SmppMessageState::EXPIRED: *pdu_ptr++=0x46;break; //expired
        case SmppMessageState::UNDELIVERABLE: *pdu_ptr++=0x41;break; //failed
        case SmppMessageState::DELETED: *pdu_ptr++=0x48;break; //failed
        default:
        {
          *pdu_ptr++=0x21;//busy
        }break;
      }
      //*pdu_ptr++=0;//0x6; //TP-Parameter-Indicator 0110
      //*pdu_ptr++=datacoding;
    }
  }
  if(!isrcpt)
  {
    if ( encoding == MAP_SMSC7BIT_ENCODING || encoding == MAP_OCTET7BIT_ENCODING  || encoding == MAP_LATIN1_ENCODING ){ // 7bit
        unsigned text_len;
        const unsigned char* text = (const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
        if(text_len==0 && sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
        {
          text=(const unsigned char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&text_len);
        }
        if ( (esm&0x40) ){
          // pdu_ptr points to ud_len,ud
          unsigned udh_len = (unsigned)*text;
          memcpy(pdu_ptr+1,text,udh_len+1);
          unsigned x = (udh_len+1)*8;
          if ( x%7 != 0 ) x+=7-(x%7);
          unsigned symbols = text_len-udh_len-1;
//          __map_trace2__("mkDeliverPDU: udh_len %d text symbols %d bit offset %d",udh_len,symbols,x-(udh_len+1)*8);
          unsigned _7bit_text_len;
          if (encoding == MAP_SMSC7BIT_ENCODING ) {
            unsigned maxlen=(unsigned)(ET96MAP_MAX_SIGNAL_INFO_LEN-(pdu_ptr+udh_len+1+1-(pdu->signalInfo+1)));
           _7bit_text_len = ConvertSMSC7bit27bit(
              text+1+udh_len,
              symbols,
              pdu_ptr+udh_len+1+1, // ud_len+1(ud)+1(udh_len)+udhlen now points to symbols
              x-(udh_len+1)*8,
              maxlen);
            *pdu_ptr++ = x/7+symbols; // x is udh len in bits filled to 7
            pdu_ptr+= udh_len+_7bit_text_len+1;
//            __map_trace2__("MAP::mkDeliverPDU: data length septets %d symbols %d octets %d",x/7+symbols,udh_len+_7bit_text_len);
          } else {
            unsigned escaped_len = 0; // escaped len in 7bit symbols
            unsigned maxlen=(unsigned)(ET96MAP_MAX_SIGNAL_INFO_LEN-(pdu_ptr+udh_len+1+1-(pdu->signalInfo+1)));
            _7bit_text_len = ConvertText27bit(
              text+1+udh_len,
              symbols,
              pdu_ptr+udh_len+1+1,
              &escaped_len,
              x-(udh_len+1)*8,
              maxlen);
            *pdu_ptr++ = x/7+escaped_len;
            pdu_ptr+= udh_len+_7bit_text_len+1;
//            __map_trace2__("MAP::mkDeliverPDU: data length septets %d symbols %d tmpX %d octets %d",x/7+escaped_len,escaped_len,udh_len+_7bit_text_len);
          }
        }else{
          if (encoding == MAP_SMSC7BIT_ENCODING ) {
           *pdu_ptr++ = text_len;
           unsigned maxlen=(unsigned)(ET96MAP_MAX_SIGNAL_INFO_LEN-(pdu_ptr-(pdu->signalInfo+1)));
            pdu_ptr += ConvertSMSC7bit27bit(text,text_len,pdu_ptr,0,maxlen);
          }
          else
          {
            unsigned escaped_len = 0; // escaped length
            unsigned maxlen=(unsigned)(ET96MAP_MAX_SIGNAL_INFO_LEN-(pdu_ptr+1-(pdu->signalInfo+1)));
            int _newbuflen = ConvertText27bit(text,text_len,pdu_ptr+1,&escaped_len,0,maxlen);
            *pdu_ptr++ = escaped_len;
            pdu_ptr += _newbuflen;
          }
        }
    }else{ // UCS2 || 8BIT
      unsigned text_len;
      const uint8_t* text;
      if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
      {
        text=(const uint8_t*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&text_len);
      }else
      {
        text=(const uint8_t*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
      }
      if ( text_len > 140 )
      {
        char *dbgtxt = new char[text_len*4];
        int k = 0;
        for ( int i=0; i<text_len; ++i)
        {
          k+=sprintf(dbgtxt+k,"%02x ",(unsigned)text[i]);
        }
        __map_warn2__("mkDeliverPDU:  UCS2 text length %d > 140: %s", text_len, dbgtxt);
        delete[] dbgtxt;
        throw runtime_error("MAP::mkDeliverPDU:  UCS2 text length > pdu_ptr-pdu->signalInfoLen");
      }
      if(encoding==8 && !HSNS_isEqual())
      {
        UCS_htons((char*)pdu_ptr+1,(const char*)text,text_len,sms->getIntProperty(Tag::SMPP_ESM_CLASS));
      }else
      {
      //unsigned size_x = /*pdu_ptr-(unsigned char*)pdu->signalInfo*;
        memcpy(pdu_ptr+1,text,text_len);
      }
      *pdu_ptr++ = text_len;
      pdu_ptr += text_len;
    }
  }
  pdu->signalInfoLen  = (uint8_t)(pdu_ptr-(unsigned char*)pdu->signalInfo);
  //if ( pdu->signalInfoLen > 140 ) header->uu.s.mms = 1;
  if( MapDialogContainer::loggerMapPdu->isDebugEnabled() ) {
    char text[sizeof(*pdu)*4] = {0,};
    int k = 0;
    for ( int i=0; i<pdu->signalInfoLen; ++i){
      k+=sprintf(text+k,"%02x ",(unsigned)pdu->signalInfo[i]);
    }
    __log2__(MapDialogContainer::loggerMapPdu,smsc::logger::Logger::LEVEL_DEBUG,"Send MAP pdu to %s from %s len=%d: %s",sms->getDestinationAddress().toString().c_str(),sms->getOriginatingAddress().toString().c_str(), pdu->signalInfoLen,text);
  }
  return pdu;
#else
  return 0;
#endif
}
