#ifndef SMSC_MCI_SME_COMPONENT
#define SMSC_MCI_SME_COMPONENT

#include <logger/Logger.h>
#include <util/Exception.hpp>

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>

#include <string>

#include "MCISmeAdmin.h"

namespace smsc {
namespace mcisme {

using namespace smsc::admin::service;

using smsc::logger::Logger;
using smsc::util::Exception;

class MCISmeComponent : public Component
{
private:

  smsc::logger::Logger *logger;

  MCISmeAdmin       &admin;
  Methods            methods;

  enum { flushStatisticsMethod, getStatisticsMethod, getRuntimeMethod, getSchedItemMethod, getSchedItemsMethod };

protected:

  void error(const char* method, const char* param);

  Variant getStatistics();
  Variant getRuntime();
  Variant getSchedItem(const std::string Abonent);
  Variant getSchedItems(void);

public:

  MCISmeComponent(MCISmeAdmin& admin);
  virtual ~MCISmeComponent() {};

  virtual const char* const getName() const {
    return "MCISme";
  }
  virtual const Methods& getMethods() const {
    return methods;
  }

  virtual Variant call(const Method& method, const Arguments& args)
    throw (AdminException);
};

}}

#endif // ifndef SMSC_MCI_SME_COMPONENT
