#ifndef __SMPPDMPLX_SMPP_MESSAGEFACTORY_HPP__
# define __SMPPDMPLX_SMPP_MESSAGEFACTORY_HPP__

# include <map>
# include <memory>

# include <logger/Logger.h>
# include <smppdmplx/SMPP_message.hpp>
# include <smppdmplx/BufferedInputStream.hpp>
# include <smppdmplx/SMPP_GenericRequest.hpp>
# include <smppdmplx/SMPP_GenericResponse.hpp>

# include <util/Singleton.hpp>

namespace smpp_dmplx {

class SMPP_MessageFactory : public smsc::util::Singleton<SMPP_MessageFactory> {
public:
  SMPP_MessageFactory();

  void registryCreatableMessage(SMPP_message* creatableMsg);

  std::auto_ptr<SMPP_message> createSmppMessage(smpp_dmplx::BufferedInputStream& ioBuf);
private:
  smsc::logger::Logger* _log;

  typedef std::map<uint32_t, SMPP_message*> registry_t;
  registry_t _cloenableMessages;

  SMPP_GenericRequest _genericRequest;
  SMPP_GenericResponse _genericResponse;
};

}

#endif
