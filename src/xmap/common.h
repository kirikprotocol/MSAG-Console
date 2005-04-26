#ifndef __COMMON_H_GREEN__
#define __COMMON_H_GREEN__

#define __FUNCTION__ __func__

struct SMS_DELIVERY_FORMAT_HEADER{
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

struct SMS_SUMBMIT_FORMAT_HEADER{
  union{
    struct{
      unsigned char reply_path:1;
      unsigned char udhi:1;
      unsigned char srr:1;
      unsigned char tp_vp:2;
      unsigned char reject_dupl:1;
      unsigned char mg_type_ind:2;
    }head;
    unsigned char _val_01;
  }u;
  unsigned char mr;
};

struct MAP_TIMESTAMP{
  struct{
    unsigned char second:4;
    unsigned char first:4;
  }year;
  struct{
    unsigned char second:4;
    unsigned char first:4;
  }mon;
  struct{
    unsigned char second:4;
    unsigned char first:4;
  }day;
  struct{
    unsigned char second:4;
    unsigned char first:4;
  }hour;
  struct{
    unsigned char second:4;
    unsigned char first:4;
  }min;
  struct{
    unsigned char second:4;
    unsigned char first:4;
  }sec;
  unsigned char tz;
};

struct MAP_SMS_ADDRESS{
 unsigned char len;
 union{
  struct{
   unsigned char reserved_1:1;
   unsigned char ton:3;
   unsigned char npi:4;
  }st;
  unsigned char tonpi;
 };
 unsigned char val[10];
};

namespace SmppMessageState{
 static const uint8_t ENROUTE      =1;
 static const uint8_t DELIVERED    =2;
 static const uint8_t EXPIRED      =3;
 static const uint8_t DELETED      =4;
 static const uint8_t UNDELIVERABLE=5;
 static const uint8_t ACCEPTED     =6;
 static const uint8_t _UNKNOWN      =7;
 static const uint8_t REJECTED     =8;
}

namespace DataCoding{
#define BIT(x) (((uint8_t)1)<<((uint8_t)x))
 static const uint8_t SMSC7BIT             = 0;
 static const uint8_t LATIN1               = 3;
 static const uint8_t BINARY               = BIT(2);
 static const uint8_t UCS2                 = BIT(3);
#undef BIT
}


#define MAP_OCTET7BIT_ENCODING 0x0
#define MAP_SMSC7BIT_ENCODING  DataCoding::SMSC7BIT
#define MAP_LATIN1_ENCODING    DataCoding::LATIN1
#define MAP_8BIT_ENCODING 0x4
#define MAP_UCS2_ENCODING 0x8

#define MSTM_WAIT_TIMEOUT 100
#define GLOBAL_SWITCHERS_TIMEOUT 100

#define MAXENTRIES 4096
#define MICROMAP_ID ETSIMAP_ID
#define GSMSC_ID    USER01_ID
#define SSN 8
#define USSD_SSN 6

#define USE_MT_SENDING
//#define CONNECT_ONCE
//////////////////////////////////////////////////////////////////////////
#define MSM_FREE 1
#define MSM_BIND_CONFIRMED 2
#define MSM_DIALOG_OPENED 3

#define uchrt  UCHAR_T
#define ushrt  USHORT_T

//////////////////////////////////////////////////////////////////////////

extern unsigned char  lll_7bit_2_8bit[128];
extern unsigned char  lll_8bit_2_7bit[256]; 

//////////////////////////////////////////////////////////////////////////




//inline void SS7ToText(  const ET96MAP_SS7_ADDR_T *addr , char* text );
//////////////////////////////////////////////////////////////////////////
//#define FULL_LOG //<-for detailed information
#define DUMP_ON

#ifdef DUMP_ON

 #define TRACE_1(fzzzzzzzzzzz,pzzzzzzz,zzzzzzlength)  int kkkkkkkk=0; char szzzsssssss[256];\
  for(int izzzzzzzz=0;izzzzzzzz<zzzzzzlength;izzzzzzzz++)\
  kkkkkkkk += sprintf(szzzsssssss+kkkkkkkk,"%02x ",pzzzzzzz[izzzzzzzz]);\
  szzzsssssss[kkkkkkkk]=0;\
  smsc_log_info(logger,fzzzzzzzzzzz,szzzsssssss)


 #define TRACE_ADDR(f,p) char text[256];\
  SS7ToText((const ET96MAP_SS7_ADDR_T *)p,(char*)&text);\
  smsc_log_info(logger,f,text)
#else

 #define TRACE_1(a,b,c){}
 #define TRACE_ADDR(a,b){}

#endif

#define DO_MAPSTATM 1
#define DO_MSHEDULER 2
#define DIALOG_FINISH 0xff
#define EMPTY_DIALOG_HASH DIALOG_FINISH-1


#define map_Initial_state 1
#define map_Opened        2
#define map_Wait_Rinfo    3
#define map_After_Rinfo   4
#define map_Wait_FwdSms   5
#define map_After_FwdSms  6
#define map_Opened_To_rx_Sms 7
#define map_After_RxSms 8


#define __assign_message_(_primitive_)  MSG_T msg;\
            msg.primitive= _primitive_;\
            msg.receiver = GSMSC_ID;\
            msg.sender   = MICROMAP_ID;

#define __send_message_()  msg.size = messmaker.getallsize(); \
          msg.msg_p = new UCHAR_T[msg.size];\
          memset(msg.msg_p,0,msg.size);\
          messmaker.getAll(msg.msg_p);\
          messmaker.free();\
          SendCPMessage(&msg);

#ifdef FULL_LOG

 #define xmap_trace smsc_log_info

#else

 #define xmap_trace

#endif

#define bad_state(st) smsc_log_info(logger,"bad dialog state %d in %s",st,__func__)


#endif