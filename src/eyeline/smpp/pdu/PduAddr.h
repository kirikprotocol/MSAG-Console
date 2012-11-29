#ifndef _EYELINE_SMPP_PDUADDR_H
#define _EYELINE_SMPP_PDUADDR_H

#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {

/*
 * SMPP 5.0 contains 3 types of addr strings:
 * 1) size=21 for submit_sm, deliver_sm and submit_multi;
 * 2) size=65 for data_sm, alert_notification
 * 3) size=41 for bind operations
 */
#define ADDRLEN21 21
#define ADDRLEN41 41
#define ADDRLEN65 65

template < size_t N >
struct addr_params_t {
    uint8_t ton;
    uint8_t npi;
    smsc::core::buffers::FixedLengthString<N> addr;

    size_t size() const {
        return sizeof(ton) + sizeof(npi) + addr.length() + 1;
    }

    void encode( eyeline::informer::ToBuf& tb ) const
    {
        tb.ncset8(ton);
        tb.ncset8(npi);
        tb.setCString(addr.c_str());
    }
    void decode( eyeline::informer::FromBuf& fb )
    {
        ton = fb.get8();
        npi = fb.get8();
        addr = fb.getCString();
    }

    static int scanNum(const char*& ptr)
    {
      int rv=0;
      while(isdigit(*ptr))
      {
        rv*=10;
        rv+=*ptr-'0';
        ++ptr;
      }
      return rv;
    }

    void setAddr(const char* text)
    {
        const char* org=text;
        if(!text || !*text)
            throw SmppException("bad address NULL");

        if(*text=='.')
        {
          ++text;
          if(!isdigit(*text))
          {
            throw SmppException("Invalid address:%s",org);
          }
          ton = scanNum(text);
          if(*text!='.')
          {
            throw SmppException("Invalid address:%s",org);
          }
          ++text;
          if(!isdigit(*text))
          {
            throw SmppException("Invalid address:%s",org);
          }
          npi = scanNum(text);
          if(*text!='.')
          {
            throw SmppException("Invalid address:%s",org);
          }
          ++text;
          if(!*text)
          {
            throw SmppException("Invalid address:%s",org);
          }
          size_t len = strlen(text);
          if( len > addr.capacity() )
          {
            throw SmppException("Address too long:%s",org);
          }
          addr.assign(text, len);
          if (ton==5)
              return;
        }else if(*text=='+')
        {
          ton = 1;
          npi = 1;
          ++text;
        }else
        {
          ton = 0;
          npi = 1;
        }
        while(*text)
        {
          if(!isdigit(*text) && *text!='?')
          {
            throw SmppException("Invalid address:%s",org);
          }
          addr += *text++;
        }
        addr += '\0';
    }
};

}
}

#endif
