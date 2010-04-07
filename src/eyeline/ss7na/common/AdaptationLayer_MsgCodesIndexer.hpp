#ifndef __EYELINE_SS7NA_COMMON_ADAPTATIONLAYERMSGCODESINDEXER_HPP__
# define __EYELINE_SS7NA_COMMON_ADAPTATIONLAYERMSGCODESINDEXER_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace ss7na {
namespace common {

class AdaptationLayer_MsgCodesIndexer {
public:
  static void registerMessageCode(uint32_t msg_code);
  static unsigned int getMessageIndex(uint32_t msg_code);

private:
  static unsigned int generateMessageIndexValue() { return ++_MSG_IDX; }

  enum { MAX_MSG_CLASS_VALUE = 9, MAX_MSG_TYPE_VALUE = 11 };
  static unsigned int _msgClassTypeToMessageIdx[MAX_MSG_CLASS_VALUE+1][MAX_MSG_TYPE_VALUE+1];
  static unsigned int _MSG_IDX;
};

}}}

#endif
