#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/sccp/SCCPAddress.hpp"

namespace eyeline {
namespace sccp {

/* ************************************************************************* *
 * class SCCPAddress implementation
 * ************************************************************************* */
//
unsigned SCCPAddress::pack2Octs(uint8_t * use_buf) const
{
  unsigned n = 0;
  use_buf[n++] = _ind.val;
  if (_ind.parm.spc) {
    use_buf[n++] = (uint8_t)_spc;         //LSB
    use_buf[n++] = (uint8_t)(_spc >> 8);  //MSB
  }
  if (_ind.parm.ssn)
    use_buf[n++] = _ssn;

  unsigned k = _gt.pack2Octs(use_buf + n);
  return k ? n + k : 0;
}

unsigned SCCPAddress::unpackOcts(const uint8_t * use_buf, unsigned buf_len)
{
  if (buf_len < 3) //at least: indicator + GT{NoA} + 1b of packed signals
    return 0;

  _ind.val = *use_buf;
  if ((buf_len - 1) < (_ind.parm.octsSize() + 1))
    return 0;

  unsigned n = 1;
  if (_ind.parm.spc) {
    _spc = (uint16_t)use_buf[n++];          //LSB
    _spc |= (uint16_t)(use_buf[n++]) << 8;  //MSB
  } else
    _spc = 0;
  if (_ind.parm.ssn)
    _ssn = use_buf[n++];
  else
    _ssn = 0;

  unsigned k = _gt.unpackOcts(_ind.parm.GTi(), use_buf + n, buf_len - n);
  return k ? n + k : 0;
}

} //sccp
} //eyeline

