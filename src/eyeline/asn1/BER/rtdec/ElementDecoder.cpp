#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/ElementDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class ElementDecoderAC implementation:
 * ************************************************************************* */

void ElementDecoderAC::insertAlternative(const EDAlternative & use_alt)
  /* throw(std::exception)*/
{
  if (!_altsArr->at(use_alt.getUId()).empty())
    throw DuplicateUIdException(use_alt.getUId());
  _altsArr->at(use_alt.getUId()) = use_alt;
  if (use_alt.isUnkExtension())
    _extensible = true;
  if (_tdState == tdEmpty)
    _tdState = tdInit;
}


ElementDecoderAC::EDAResult
  ElementDecoderAC::processElement(const ASTag & use_tag) /*throw(std::exception)*/
{
  if (_tdState == tdInit) {
    buildTagDecoder();
    _tdState = tdBuilt;
  }
  _tdState = tdProcess;
  return processElementTag(use_tag);
}

//Reverts ElementDecoderAC to its initialized state, making it ready for next decoding.
void ElementDecoderAC::reset(void) /*throw()*/
{
  if (_tdState == tdProcess) {
    resetTagDecoder();
    _tdState = tdBuilt;
  }
}

//ElementDecoderAC to its 'just-allocated' state, erases all runtime and
//initialization data.
void ElementDecoderAC::erase(void) /*throw()*/
{
  while (_tdState > tdEmpty) {
    switch (_tdState) {
    case tdProcess: {
      resetTagDecoder(); _tdState = tdBuilt; 
    } break;

    case tdBuilt: {
      eraseTagDecoder();  _tdState = tdInit;
    } break;

    case tdInit: {
      _altsArr->clear();
      _extensible =  false;
      _tdState = tdEmpty;
    } break;

    default:;
    } //eosw
  }
}


} //ber
} //asn1
} //eyeline

