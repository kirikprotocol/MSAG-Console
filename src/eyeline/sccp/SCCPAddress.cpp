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
  use_buf[n++] = _ind.pack2Oct();
  if (_ind.hasSPC) {
    use_buf[n++] = (uint8_t)_spc;         //LSB
    use_buf[n++] = (uint8_t)(_spc >> 8);  //MSB
  }
  if (_ind.hasSSN)
    use_buf[n++] = _ssn;

  unsigned k = _gt.pack2Octs(use_buf + n);
  return k ? n + k : 0;
}

unsigned SCCPAddress::unpackOcts(const uint8_t * use_buf, unsigned buf_len)
{
  if (buf_len < 3) //at least: indicator + GT{NoA} + 1b of packed signals
    return 0;

  _ind.unpackOct(*use_buf);
  if ((buf_len - 1) < (_ind.octsSize() + 1))
    return 0;

  unsigned n = 1;
  if (_ind.hasSPC) {
    _spc = (uint16_t)use_buf[n++];          //LSB
    _spc |= (uint16_t)(use_buf[n++]) << 8;  //MSB
  } else
    _spc = 0;
  if (_ind.hasSSN)
    _ssn = use_buf[n++];
  else
    _ssn = 0;

  unsigned k = _gt.unpackOcts(_ind.gti, use_buf + n, buf_len - n);
  return k ? n + k : 0;
}

#define CHECK_RES(pr_res, pr_cnt, buf_len) \
  if ((pr_res > (buf_len - pr_cnt)) || (pr_res < 1)) \
    return pr_cnt; pr_cnt += pr_res;

//".route(SSN).ssn(uuu).spc(uuuuu).GT"
size_t  SCCPAddress::toString(char * use_buf, size_t max_len) const
{
  size_t rval = 0;

  int n = snprintf(use_buf, max_len, ".route(%s)", _ind.ssnRouted ? "SSN" : "SPC");
  CHECK_RES(n, rval, max_len);

  if (_ind.hasSSN) {
    n = snprintf(use_buf + rval, max_len - rval, ".ssn(%u)", _ssn);
    CHECK_RES(n, rval, max_len);
  }
  if (_ind.hasSPC) {
    n = snprintf(use_buf + rval, max_len - rval, ".spc(%u)", _spc);
    CHECK_RES(n, rval, max_len);
  }
  rval += _gt.toString(use_buf + rval, max_len - rval);
  return rval;
}

} //sccp
} //eyeline

