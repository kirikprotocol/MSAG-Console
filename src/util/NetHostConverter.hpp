/* ************************************************************************** *
 * Network to host byte order converter for native C99 integer types.
 * ************************************************************************** */
#ifndef __UITL_NET_HOST_BYTEORDER_CONVERTER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UITL_NET_HOST_BYTEORDER_CONVERTER

namespace smsc  {
namespace util  {

template < typename _IntT >
struct AlignedInt_T {
  union {
    unsigned char   _buf[sizeof(_IntT)];
    _IntT           _ival;
  };

  explicit AlignedInt_T(_IntT use_val = 0)
  {
    _ival = use_val;
  }
};

//Helper determining host byte order while runtime.
class HostByteOrder {
protected:
  AlignedInt_T<unsigned short> u;

public:
  HostByteOrder() : u(0x1234)
  { }

  bool isBigEndian(void) const { return u._buf[0] == 0x12; }
  bool isLittleEndian(void) const { return u._buf[0] == 0x34; }
  bool isNetworkOrder(void) const { return isBigEndian(); }
};


class NetHostConverter {
protected:
  static void swapOctets(unsigned char * use_buf, unsigned char buf_sz)
  {
    unsigned char maxI = buf_sz-- >> 1;
    for (unsigned char i = 0; i < maxI; ++i, --buf_sz) {
      unsigned char tmp = use_buf[i];
      use_buf[i] = use_buf[buf_sz];
      use_buf[buf_sz] = tmp;
    }
  }

public:
  static bool isHostOrderEqualToNetwork(void)
  {
    static HostByteOrder  _hostOrder;
    return _hostOrder.isNetworkOrder();
  }

  template < typename _IntT >
  static _IntT swapInt(_IntT use_val)
  {
    AlignedInt_T<_IntT> u;
    u._ival = use_val;
    swapOctets(u._buf, (unsigned char)sizeof(u._buf));
    return u._ival;
  }

  template < typename _IntT >
  static _IntT toHostOrder(_IntT use_val)
  {
    return isHostOrderEqualToNetwork() ? use_val : swapInt(use_val);
  }

  template < typename _IntT >
  static _IntT toNetworkOrder(_IntT use_val)
  {
    return isHostOrderEqualToNetwork() ? use_val : swapInt(use_val);
  }
};

} //util
} //smsc
#endif /* __UITL_NET_HOST_BYTEORDER_CONVERTER */

