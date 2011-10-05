#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <time.h>
#include "core/buffers/Hash.hpp"
#include "logger/Logger.h"

#include <openssl/crypto.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#ifdef linux
#include <dbus/dbus.h>
#endif

namespace smsc{
namespace license{
namespace check{

using namespace std;
using smsc::core::buffers::Hash;
/*
static const char *lkeys[]=
{
"Organization",
"Hostids",
"MaxSmsThroughput",
"LicenseExpirationDate",
"LicenseType",
"Product"
};
*/


static bool ReadFile(const char* fn,string& str)
{
  FILE* f=fopen(fn,"rb");
  if(!f)return false;
  fseek(f,0,SEEK_END);
  size_t sz=ftell(f);
  fseek(f,0,SEEK_SET);

  vector<char> v(sz,0);
  fread(&v[0],sz,1,f);
  str.assign(v.begin(),v.end());
  fclose(f);
  return true;
}

static void trim(char* buf)
{

  size_t st=0,fn=strlen(buf)-1;
  while(st<fn && isspace(buf[st]))st++;
  while(fn>0 && isspace(buf[fn]))fn--;
  if(fn==0 && isspace(buf[fn]))
  {
    buf[0]=0;
    return;
  }
  if(st==0)
  {
    buf[fn+1]=0;
    return;
  }
  memmove(buf,buf+st,fn-st+1);
  buf[fn-st+1]=0;
}

bool ReadLicense(FILE* f,Hash<string>& lic)
{
  char buf[4096];
  char nm[4096];
  char vl[4096];
  while(!feof(f))
  {
    if(!fgets(buf,(int)sizeof(buf),f))break;
    trim(buf);
    if(!buf[0] || buf[0]=='#')continue;
    char *eq=strchr(buf,'=');
    if(!eq)return false;
    if(eq-buf>4000)
    {
      return false;
    }
    memcpy(nm,buf,eq-buf);
    nm[eq-buf]=0;
    trim(nm);
    if(strlen(eq+1)>4000)
    {
      return false;
    }
    strcpy(vl,eq+1);
    trim(vl);
    lic.Insert(nm,vl);
  }
  return true;
}

bool CheckLicense(const char* lf,const char* sig,Hash<string>& lic,const char* keys[],size_t keysCount)
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.license");

  /*
  BIO* in=BIO_new(BIO_s_file());
  if (BIO_read_filename(in,pk) <= 0)
  {
    log.error("error: failed to open public key file");
    return false;
  }
  RSA *rsa=PEM_read_bio_RSA_PUBKEY(in,NULL, NULL,NULL);
  if(!rsa)
  {
    log.error("error: failed to read public key");
    return false;
  }
  */
  RSA *rsa=NULL;
  static unsigned char rsapublickey[]={0x30,0x82,0x02,0x08,0x02,0x82,0x02,0x01,0x00,0xC4,0xF1,0x44,0x34,0x08,0xD7,0x03,0xA3,0x6C,0xBF,0x8F,0xF8,0x48,0xCF,0x8D,0xEF,0xBC,0x34,0x8D,0xAE,0x66,0x49,0x83,0xAC,0x99,0x94,0x7E,0xC5,0x44,0x22,0xC1,0x44,0xCD,0xD2,0xCF,0xF6,0xF6,0x4D,0x07,0xBA,0x07,0x70,0x5E,0x49,0x38,0xC3,0xD1,0xD2,0x23,0xBA,0xC8,0xAD,0x0C,0x5A,0x7D,0x19,0xD9,0x1D,0x9F,0xA3,0x32,0x0C,0x1D,0x15,0xC5,0x32,0xAF,0x21,0xBD,0x73,0xA3,0xC9,0x2D,0x8A,0x78,0x0D,0x6D,0x19,0x4D,0x74,0x2E,0x9F,0xF1,0xDD,0x06,0x6D,0x6D,0x39,0xAB,0xB7,0x9E,0xF7,0xF9,0xFB,0xBB,0xCD,0xFB,0xBD,0x64,0xBE,0x44,0x1E,0xA0,0x3C,0xEF,0x9D,0xB5,0xE2,0x60,0x28,0xDC,0x4E,0xF3,0x9B,0x09,0xBD,0x31,0x30,0xCD,0x83,0x12,0xEB,0x9C,0xAE,0x5F,0xDD,0x7D,0x71,0x4E,0x33,0xCA,0x6D,0x96,0x3D,0x8B,0xD0,0x0F,0x99,0x8C,0xE8,0xD1,0x2B,0xED,0xA4,0x3B,0x20,0x78,0x23,0xDF,0xFB,0x10,0xD4,0x99,0x2D,0x8B,0xF1,0x8D,0x0A,0xB2,0x09,0xB2,0x1C,0xE4,0x58,0xAC,0x38,0xD7,0xE1,0xBD,0x24,0x48,0x99,0x27,0x00,0xCF,0x30,0x09,0xD8,0x7E,0x80,0x7C,0x11,0xC6,0xCC,0x41,0x5F,0x96,0xD4,0x33,0xEC,0x85,0x7B,0x27,0xA8,0xB0,0x84,0xC5,0x63,0x6B,0x3C,0xD2,0x27,0x9C,0x24,0x35,0x18,0x49,0xD4,0x35,0xAA,0x88,0x4D,0x42,0x30,0xAF,0x11,0x9A,0xCB,0x5D,0xAC,0x40,0xD4,0x89,0xB5,0x28,0x64,0xE3,0x6D,0xF2,0x75,0x59,0x32,0x59,0x2F,0x81,0x7D,0x6B,0xF9,0xCC,0x36,0xB5,0xFD,0xC8,0x05,0xA3,0x09,0x6C,0xC6,0x65,0x4D,0x84,0x6D,0x77,0x71,0xD4,0x93,0xDF,0xE7,0x57,0xB5,0x2B,0xB6,0xE7,0x41,0x3B,0xEC,0xD3,0xA6,0x2C,0xF0,0xC4,0x47,0xE3,0x79,0x95,0xD6,0x6A,0xD8,0xBE,0xE9,0x2C,0xE0,0x03,0x76,0xDA,0x75,0x4B,0xB7,0x97,0x92,0x55,0x07,0x24,0xEE,0xE7,0xB7,0xC4,0xFB,0x16,0x0A,0x38,0x42,0x3C,0x40,0x4F,0x92,0x68,0xC1,0x48,0x9A,0x7D,0xD2,0xB5,0xF2,0x8E,0xCA,0xA8,0x38,0x5A,0x20,0x61,0x2A,0x27,0xFD,0xC1,0xAA,0xEF,0x8F,0xFC,0x97,0xFB,0xAD,0x0B,0xA1,0x00,0x72,0x51,0xD9,0x6B,0xAA,0x41,0x34,0x8B,0xD5,0xBD,0x2A,0xEF,0x38,0x13,0xB4,0xB9,0x3A,0xA7,0xC8,0x2C,0xDB,0x42,0xAC,0xEA,0x36,0x78,0xC3,0x71,0x8C,0x94,0x3D,0xF7,0x9E,0xA3,0xDB,0x35,0x4B,0x0D,0x8C,0xA3,0x88,0xFA,0xED,0x4B,0x5E,0xA6,0x02,0x5F,0xF9,0x39,0x6D,0x65,0xF5,0x11,0x4D,0xEB,0x54,0xDE,0x2B,0x7F,0xC7,0xFE,0x49,0x43,0x1B,0x0E,0x26,0xEC,0xAA,0x9F,0xE1,0xD1,0x43,0xB0,0xD3,0xF3,0x84,0x28,0x92,0x45,0x00,0xF2,0xCB,0x70,0xE3,0xD8,0x45,0xB8,0x20,0xE7,0xCC,0x9B,0x07,0xC6,0xDC,0x74,0x49,0xB8,0x61,0x3D,0x5F,0xED,0xA1,0xCF,0x1F,0x2D,0xB0,0x0D,0x1A,0xF6,0xC1,0x6A,0x3A,0x04,0x91,0xCF,0x47,0x79,0x81,0xD7,0xB9,0x1B,0x12,0x61,0x89,0x58,0xA1,0x0B,0x2C,0x94,0xD4,0xE5,0xA5,0x32,0x3D,0x73,0x22,0xEC,0x78,0x5E,0xF8,0xDC,0xB9,0x8A,0x6F,0x7F,0x9E,0x99,0x5A,0xA2,0xC5,0xC9,0xA9,0x15,0x15,0x6B,0x5F,0xC9,0x63,0xCA,0xAF,0x0B,0xB9,0x40,0x1F,0x62,0x68,0x4E,0x3D,0x47,0x5E,0x20,0x1D,0xF2,0xA7,0x61,0xB3,0x02,0x01,0x11};
  const unsigned char *p=rsapublickey;
  d2i_RSAPublicKey(&rsa,&p,sizeof(rsapublickey));
  if(!rsa)
  {
    smsc_log_error(log, "error: failed to read public key");
    return false;
  }

  string sign;
  if(!ReadFile(sig,sign))
  {
    smsc_log_error(log, "error: failed to read signature file");
    return false;
  }
  vector<unsigned char> signbuf;
  for(unsigned i=0;i<sign.length();i+=2)
  {
    unsigned int x;
    while(sign[i] && !isalnum(sign[i]))i++;
    sscanf(&sign[i],"%02X",&x);
    signbuf.push_back(x);
  }

  FILE* f=fopen(lf,"rb");
  if(!f)
  {
    smsc_log_error(log, "error: failed to open license file!");
    return false;
  }
  if(!ReadLicense(f,lic))
  {
    fclose(f);
    smsc_log_error(log, "error: failed to read license file!");
    return false;
  }
  fclose(f);


  string msg;
  for(size_t i=0;i<keysCount;i++)
  {
    msg+=keys[i];
    msg+='=';
    if(!lic.Exists(keys[i]))
    {
      smsc_log_error(log, "Field %s not found in license",keys[i]);
      return false;
    }
    msg+=lic[keys[i]];
    msg+=';';
  }

  unsigned char md[SHA_DIGEST_LENGTH];
  unsigned int mdsz=SHA_DIGEST_LENGTH;
  SHA1((const unsigned char*)msg.c_str(),msg.length(),md);

  if(!RSA_verify(NID_sha1,(unsigned char*)md,mdsz,&signbuf[0],(unsigned)signbuf.size(),rsa))
  {
    lic.Empty();
    return false;
  }
  smsc_log_info(log, "Licensed for %s",lic["Organization"].c_str());
  smsc_log_info(log, "License type %s",lic["LicenseType"].c_str());
  smsc_log_info(log, "License expiration date %s",lic["LicenseExpirationDate"].c_str());
  smsc_log_info(log, "Licensed for host ids %s",lic["Hostids"].c_str());
  smsc_log_info(log, "Licensed maximum sms throughput %s",lic["MaxSmsThroughput"].c_str());
  return true;
}


void gethostid( char* buf, size_t buflen )
{
#ifdef linux
    // read from /org/freedesktop/Hal/devices/computer : 'system.hardware.uuid'
    const char* what = 0;
    DBusConnection* dbus = 0;
    DBusMessage* msg = 0;
    do {

        struct DBusErrorGuard {
            DBusErrorGuard() {
                dbus_error_init(&e);
            }
            ~DBusErrorGuard() {
                dbus_error_free(&e);
            }
            inline void free() {
                dbus_error_free(&e);
            }
            inline bool is_set() {
                return dbus_error_is_set(&e);
            }
            DBusError e;
        } err;

        // connect
        dbus = dbus_bus_get_private(DBUS_BUS_SYSTEM,&err.e);
        if ( err.is_set() ) {
            fprintf(stderr,"Connection error (%s)\n",err.e.message);
            err.free();
        }
        if (0 == dbus) {
            what = "cannot connect to dbus";
            break;
        }

        // request name
        /*
        dbus_bus_request_name( dbus, "test.method.client.scag",
                               DBUS_NAME_FLAG_REPLACE_EXISTING,
                               &err.e );
        if ( err.is_set() ) {
            fprintf(stderr,"Name error (%s)\n",err.e.message);
            err.free();
        }
        if ( DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret ) {
            what = "cannot set dbus name";
            break;
        }
         */

        // create a message
        msg = dbus_message_new_method_call( "org.freedesktop.Hal",
                                            "/org/freedesktop/Hal/devices/computer",
                                            "org.freedesktop.Hal.Device",
                                            "GetProperty" );
        if ( 0 == msg ) {
            what = "cannot create dbus msg";
            break;
        }

        DBusMessageIter args;
        dbus_message_iter_init_append(msg,&args);
        const char* stringname = "system.hardware.uuid";
        if ( !dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&stringname)) {
            what = "out of memory on msg args";
            break;
        }

        // send message
        DBusPendingCall* pending;
        if ( !dbus_connection_send_with_reply( dbus, msg, &pending, -1 )) {
            what = "out of memory on send";
            break;
        }
        if ( 0 == pending ) {
            what = "pending call null";
            break;
        }
        dbus_connection_flush(dbus);
        dbus_message_unref(msg); msg = 0;

        dbus_pending_call_block(pending);
        msg = dbus_pending_call_steal_reply(pending);
        dbus_pending_call_unref(pending);
        if ( 0 == msg ) {
            what = "reply null";
            break;
        }

        if ( !dbus_message_iter_init(msg,&args) ) {
            fprintf(stderr,"reply has no arguments\n");
            break;
        }

        /*
        do {

            const int argtype = dbus_message_iter_get_arg_type(&args);
            switch (argtype) {
            case DBUS_TYPE_BOOLEAN: {
                dbus_bool_t res;
                dbus_message_iter_get_basic(&args,&res);
                fprintf(stderr,"bool %d\n",res);
                break;
            }
            case DBUS_TYPE_UINT32: {
                dbus_uint32_t res;
                dbus_message_iter_get_basic(&args,&res);
                fprintf(stderr,"uint32 %u\n",unsigned(res));
                break;
            }
            case DBUS_TYPE_STRING : {
                char* res;
                dbus_message_iter_get_basic(&args,&res);
                fprintf(stderr,"string %s\n",res);
                break;
            }
            default:
                fprintf(stderr,"unknown type: %d\n",argtype);
            }

        } while ( dbus_message_iter_next(&args) );
         */

        if ( DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args) ) {
            what = "dbus result is not string";
            break;
        }

        char* hi;
        dbus_message_iter_get_basic(&args,&hi);
        strncpy(buf,hi,buflen);
        buf[buflen-1] = '\0';

        if ( dbus_message_iter_next(&args) ) {
            what = "dbus result has spurious data";
            break;
        }

    } while (false);

    if (msg) { dbus_message_unref(msg); }
    if (dbus) { dbus_connection_close(dbus); }

    if (what) {
        throw smsc::util::Exception("exc in hostid: %s",what);
    }
#else
    const long hi = ::gethostid();
    snprintf(buf,buflen,"%08lx",hi);
#endif
}


bool checkHostIds( const char* hostids )
{
    char hostid[200];
    smsc::license::check::gethostid( hostid, sizeof(hostid) );
    for ( const char* p = hostids; *p != '\0'; ) {
        while ( *p == ' ' || *p == '\t' || *p == ',' ) {
            ++p;
        }
        if ( *p == '\0' ) break;
        char buf[200];
        int pos = 0;
        sscanf(p,"%150[0-9a-fA-F-]%n",buf,&pos);
        if (!pos) {
            throw std::runtime_error("wrong hostids");
        }
        p += pos;
        if ( 0 == strcmp(buf,hostid) ) {
            return true;
        }
    }
    return false;
}


}//namespace check
}//namespace license
}//namespace smsc
