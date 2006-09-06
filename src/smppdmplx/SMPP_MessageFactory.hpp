#ifndef __SMPPDMPLX_SMPP_MESSAGEFACTORY_H__
# define __SMPPDMPLX_SMPP_MESSAGEFACTORY_H__ 1

# include "SMPP_message.hpp"
# include "BufferedInputStream.hpp"

# include <list>
# include <memory>

# include <util/Singleton.hpp>

namespace smpp_dmplx {

class SMPP_MessageFactory : public smsc::util::Singleton<SMPP_MessageFactory> {
public:
  void registryCreatableMessage(SMPP_message* creatableMsg);

  std::auto_ptr<SMPP_message> createSmppMessage(smpp_dmplx::BufferedInputStream& ioBuf);
private:
  std::list<SMPP_message*> _creatableMessageList;
};

}

#endif
