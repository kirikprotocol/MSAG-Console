#ifndef __EYELINE_MAP_7F0_PROTO_ERR_CALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_CALLBARREDPARAM_HPP__

# include <inttypes.h>
# include "eyeline/util/MaxSizeof.hpp"
# include "eyeline/map/7_15_0/proto/ERR/CallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"

namespace eyeline {
namespace map {
namespace err {

/*
 CallBarredParam ::= CHOICE {
   callBarringCause        CallBarringCause,
   -- call BarringCause must not be used in version 3 and higher
   extensibleCallBarredParam       ExtensibleCallBarredParam
   -- extensibleCallBarredParam must not be used in version <3
 }
 */
class CallBarredParam {
public:
  enum Kind_e { KindNone, KindCallBarringCause, KindExtensibleCallBarredParam };

  CallBarredParam()
  : _kind(KindNone)
  {
    _choice.extensibleCallBarredParam= NULL;
  }

  ~CallBarredParam() {
    clear();
  }

  CallBarringCause& initCallBarringCause() {
    clear();
    _kind= KindCallBarringCause;
    _choice.callBarringCause= new (_memAlloc.buf) CallBarringCause();
    return *_choice.callBarringCause;
  }

  ExtensibleCallBarredParam& initExtensibleCallBarredParam() {
    clear();
    _kind= KindExtensibleCallBarredParam;
    _choice.extensibleCallBarredParam= new (_memAlloc.buf) ExtensibleCallBarredParam();
    return *_choice.extensibleCallBarredParam;
  }

  Kind_e getKind() const { return _kind; }

  const CallBarringCause* getCallBarringCause() const {
    return (_kind == KindCallBarringCause) ? _choice.callBarringCause : NULL;
  }
  const ExtensibleCallBarredParam* getExtensibleCallBarredParam() const {
    return (_kind == KindExtensibleCallBarredParam) ? _choice.extensibleCallBarredParam : NULL;
  }

  CallBarringCause* getCallBarringCause() {
    return (_kind == KindCallBarringCause) ? _choice.callBarringCause : NULL;
  }
  ExtensibleCallBarredParam* getExtensibleCallBarredParam() {
    return (_kind == KindExtensibleCallBarredParam) ? _choice.extensibleCallBarredParam : NULL;
  }

protected:
  void clear() {
    if (_kind == KindCallBarringCause)
      _choice.callBarringCause->~CallBarringCause();
    else if (_kind == KindExtensibleCallBarredParam)
      _choice.extensibleCallBarredParam->~ExtensibleCallBarredParam();
    _kind= KindNone;
    _memAlloc.aligner= NULL;
    _choice.extensibleCallBarredParam= NULL;
  }

private:
  union {
    void * aligner;
    uint8_t buf[util::MaxSizeOf2_T<ExtensibleCallBarredParam, CallBarringCause>::VALUE];
  } _memAlloc;

  union {
    CallBarringCause* callBarringCause;
    ExtensibleCallBarredParam* extensibleCallBarredParam;
  } _choice;

  Kind_e _kind;
};

}}}

#endif
