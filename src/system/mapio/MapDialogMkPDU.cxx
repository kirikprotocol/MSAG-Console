#pragma pack(1)

struct SMS_DELIVERY_FORMAT_HEADER{
  union{                          
    struct{                       
      unsigned reply_path:1;      
      unsigned udhi:1;            
      unsigned srri:1;            
      unsigned reserved:2;        
      unsigned mms:1;             
      unsigned mg_type_ind:2;     
    }s;                           
    unsigned char _val_01;        
  }uu;                            
}; 

#pragma pack()

ET96MAP_SM_RP_UI_T* mkDeliverPDU(SMS* sms,ET96MAP_SM_RP_UI_T* pdu)
{
#if defined USE_MAP
  memset(pdu,0,sizeof(ET96MAP_SM_RP_UI_T));
  SMS_DELIVERY_FORMAT_HEADER* header = (SMS_DELIVERY_FORMAT_HEADER*)pdu->signalInfo;
  header->uu.s.mg_type_ind = 0;
  header->uu.s.mms = 0;
  header->uu.s.reply_path = (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x80)?1:0;;
  header->uu.s.srri = 0;
  header->uu.s.udhi = (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)?1:0;
  MAP_SMS_ADDRESS* oa = (MAP_SMS_ADDRESS*)(pdu->signalInfo+1);
  oa->st.ton = sms->getOriginatingAddress().getTypeOfNumber();
  oa->st.npi = sms->getOriginatingAddress().getNumberingPlan();
  oa->st.reserved_1 = 1;
  oa->len = sms->getOriginatingAddress().getLength();
  unsigned oa_length = (oa->len+1)/2; 
  __trace2__("MAP::mkDeliverPDU: oa_length: 0x%x", oa_length);
  {
    char* sval = sms->getOriginatingAddress().value;
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
  unsigned char *pdu_ptr = pdu->signalInfo+1+2+oa_length;
  *pdu_ptr++ = (unsigned char)sms->getIntProperty(Tag::SMPP_PROTOCOL_ID);
  unsigned encoding = sms->getIntProperty(Tag::SMPP_DATA_CODING);
  __trace2__("MAP::mkDeliveryPDU: encoding = 0x%x",encoding);
  unsigned char value;
  if ( encoding != 0 && 
       encoding != 0x08 && 
       encoding != 0x03 && 
       encoding != 0x04 && 
       encoding != 0xf0) {
    __trace2__("MAP::mkDeliverPDU: unsuppprted encoding 0x%x",encoding);
    throw runtime_error("unsupported encoding");
  }
  else // make coding scheme
  {
    if ( sms->hasIntProperty(Tag::MS_DESTADDRSUBUNIT) ){
      __trace2__("MAP::mkDeliveryPDU: dest_addr_subunit = 0x%x",
                 sms->getIntProperty(Tag::MS_DESTADDRSUBUNIT));
      // coding scheme 1111xxxx
      value = 0xf0;
      if ( encoding == MAP_UCS2_ENCODING){
        __trace2__("MAP::mkDeliveryPDU: coding group 1111xxxx could'not has USC2");
        throw runtime_error("MAP::mkDeliveryPDU: coding group 1111xxxx incompatible with encoding UCS2");
      }
      if ( encoding == MAP_OCTET7BIT_ENCODING || encoding == MAP_LATIN1_ENCODING )
        ;//value |=  nothing
      else // 8bit
        value |= (1<<2);
      value |= sms->getIntProperty(Tag::MS_DESTADDRSUBUNIT)&0x3;
    }
    else
    {
      if ( sms->hasIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES) ){
        __trace2__("MAP::mkDeliveryPDU: ms_msg_wait_facilities = 0x%x",
                   sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES));
        if ( encoding == MAP_UCS2_ENCODING ){
          value = 0xe0;
          unsigned _val = sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES);
          value |= _val&0x3;
          if ( _val&0x80 )value |= 0x8;
        }
        else
        {
          if ( !sms->hasIntProperty(Tag::MS_VALIDITY) ){
            __trace2__("MAP::mkDeliveryPDU: Opss, has no ms_validity");
            throw runtime_error("MAP::mkDeliveryPDU: Opss, has no ms_validity");
          }
          unsigned ms_validity = sms->getIntProperty(Tag::MS_VALIDITY);
          __trace2__("MAP::mkDeliveryPDU: ms_validity = 0x%x",
                     ms_validity);
          if ( (ms_validity & 0x3) == 0x3 ){
            value = 0xc0;
          }else if ( (ms_validity & 0x3) == 0 ){
            value = 0xd0;
          }else{
            __trace2__("MAP::mkDeliveryPDU: Opss, ms_validity = 0x%x but must be 0x0 or 0x3",
                       ms_validity);
            throw runtime_error("bad ms_validity value");
          }
          unsigned _val = sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES);
          value |= _val&0x3;
          if ( _val&0x80 )value |= 0x8;
        }
      }
      else
      {
        /*if ( !sms->hasIntProperty(Tag::MS_VALIDITY) ){
          __trace2__("MAP::mkDeliveryPDU: Opss, has no ms_validity");
          throw runtime_error("MAP::mkDeliveryPDU: Opss, has no ms_validity");
        }*/
        unsigned ms_validity = 0;
        if ( sms->hasIntProperty(Tag::MS_VALIDITY) )
          ms_validity = sms->getIntProperty(Tag::MS_VALIDITY);
        __trace2__("MAP::mkDeliveryPDU: ms_validity = 0x%x",
                   ms_validity);
        if ( (ms_validity & 0x3) == 0x3 ){
          __trace2__("MAP::mkDeliveryPDU: (validity & 0x3) == 0x3");
          value = 0x40;
        }else if ( (ms_validity & 0x3) == 0 ){
          __trace2__("MAP::mkDeliveryPDU: (validity & 0x3) == 0");
          value = 0x00;
        }else{
          __trace2__("MAP::mkDeliveryPDU: Opss, ms_validity = 0x%x but must be 0x0 or 0x3",
                     ms_validity);
          throw runtime_error("bad ms_validity value");
        }
        if ( encoding == MAP_UCS2_ENCODING ){
          __trace2__("MAP::mkDeliveryPDU: MAP_UCS2_ENCODING");
           value |= 0x8;
        }
        //if ( encoding == 0 && encoding == 0x3 ) nothing
        if ( encoding == MAP_8BIT_ENCODING ){
          __trace2__("MAP::mkDeliveryPDU: MAP_8BIT_ENCODING");
           value |= 0x4;
        }
      }
    }
    __trace2__("MAP::mkDeliveryPDU: user data coding = 0x%x",(unsigned)value);
    *pdu_ptr++ = value;
  }
  {
    time_t t;
    time(&t);
    struct tm* tms = localtime(&t);  
    MAP_TIMESTAMP* pdu_tm = (MAP_TIMESTAMP*)pdu_ptr;
    __trace2__("MAP::mkDeliverPDU:sizeof(pdu_tm) %d",sizeof(*pdu_tm));
    pdu_tm->year.first  =  ((tms->tm_year)%100)/10;
    pdu_tm->year.second  = tms->tm_year%10;
    //__trace2__("MAP::mkDeliverPDU: year: 0x%x, tms_year 0x%lx",pdu_tm->year,tms->tm_year);
    pdu_tm->mon.first  =  (tms->tm_mon+1)/10;
    pdu_tm->mon.second  = (tms->tm_mon+1)%10;
    pdu_tm->day.first  =  tms->tm_mday/10;
    pdu_tm->day.second  = tms->tm_mday%10;
    pdu_tm->hour.first  =  tms->tm_hour/10;
    pdu_tm->hour.second  = tms->tm_hour%10;
    pdu_tm->min.first  =  tms->tm_min/10;
    pdu_tm->min.second  = tms->tm_min%10;
    pdu_tm->sec.first  =  tms->tm_sec/10;
    pdu_tm->sec.second  = tms->tm_sec%10;
    int tz = timezone;
    if ( tms->tm_isdst ) tz-=3600;
    tz = -tz/900;
    __trace2__("MAP::mkDeliverPDU: timezone %d, %ld",tz,timezone);
    pdu_tm->tz = tz;
    pdu_ptr+=sizeof(MAP_TIMESTAMP);
  }
  if ( encoding == MAP_OCTET7BIT_ENCODING  || encoding == MAP_LATIN1_ENCODING ){
    unsigned text_len;
    const unsigned char* text = (const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
    unsigned elen;
    unsigned bytes;
    bytes = ConvertText27bit(text,text_len,pdu_ptr+1,&elen);
    *pdu_ptr++ = elen;
    pdu_ptr += bytes;
  }else if ( encoding == MAP_SMSC7BIT_ENCODING  ){ // 7bit
      unsigned text_len;
      const unsigned char* text = (const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
      if ( header->uu.s.udhi ){
        unsigned udh_len = (unsigned)*text;
        __trace2__("MAP::mkDeliverPDU: udh_len 0x%x",udh_len);
        memcpy(pdu_ptr+1,text,udh_len+1);
        unsigned x = (udh_len+1)*8;
        if ( x%7 != 0 ) x+=7-(x%7);
        unsigned symbols = text_len-udh_len-1;
        __trace2__("MAP::mkDeliverPDU: text symbols 0x%x",symbols);
        __trace2__("MAP::mkDeliverPDU: text bit offset 0x%x",x-(udh_len+1)*8);
        unsigned _7bit_text_len = ConvertSMSC7bit27bit(
          text+1+udh_len,
          symbols,
          pdu_ptr+udh_len+1+1,
          x-(udh_len+1)*8);
        *pdu_ptr++ = x/7+text_len+1;
        pdu_ptr+= udh_len+_7bit_text_len+1;
      }else{
        *pdu_ptr++ = text_len;
        pdu_ptr += ConvertSMSC7bit27bit(text,text_len,pdu_ptr);
      }
  }else{ // UCS2 || 8BIT
    unsigned text_len;
    const unsigned char* text = (const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
    //unsigned size_x = /*pdu_ptr-(unsigned char*)pdu->signalInfo*;
    if ( text_len > 140 ){
      __trace2__("MAP::mkDeliverPDU:  UCS2 text length(%d) > 140",
                text_len);
      throw runtime_error("MAP::mkDeliverPDU:  UCS2 text length > pdu_ptr-pdu->signalInfoLen");
    }
    memcpy(pdu_ptr+1,text,text_len);
    *pdu_ptr++ = text_len;
    pdu_ptr += text_len;
  }
  pdu->signalInfoLen  = pdu_ptr-(unsigned char*)pdu->signalInfo;
  //if ( pdu->signalInfoLen > 140 ) header->uu.s.mms = 1;
  __trace2__("MAP::mkDeliverPDU: signalInfoLen 0x%x",pdu->signalInfoLen);
  {
    char text[sizeof(*pdu)*4] = {0,};
    int k = 0;
    for ( int i=0; i<pdu->signalInfoLen; ++i){
      k+=sprintf(text+k,"%02x ",(unsigned)pdu->signalInfo[i]);
    }
    __trace2__("MAP::mkDeliverPDU: PDU %s",text);
  }
  return pdu;
#else
  return 0;
#endif
}
