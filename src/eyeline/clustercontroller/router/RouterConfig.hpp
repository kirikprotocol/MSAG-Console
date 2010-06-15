/*
 * RouterConfig.hpp
 *
 *  Created on: May 11, 2010
 *      Author: skv
 */

#ifndef __EYELINE_CLUSTERCONTROLLER_ROUTER_ROUTERCONFIG_HPP__
#define __EYELINE_CLUSTERCONTROLLER_ROUTER_ROUTERCONFIG_HPP__

#include "smsc/smeman/smetable.h"
#include "smsc/smeman/smeadmin.h"
#include "core/buffers/RefPtr.hpp"
#include "core/synchronization/Mutex.hpp"
#include <string>

namespace smsc{
namespace router{
class RouteManager;
}
}

namespace eyeline {
namespace clustercontroller {
namespace router {

class RouterConfig
{
public:
  typedef smsc::core::buffers::RefPtr<smsc::router::RouteManager,smsc::core::synchronization::Mutex> RouterRef;
protected:
  static RouterRef instance;
  static std::string routerConfig;
  static std::string smeConfig;

  struct SmeTableImpl:public smsc::smeman::SmeTable, public smsc::smeman::SmeAdministrator{
    typedef std::vector<smsc::smeman::SmeInfo> SmeVector;
    SmeVector table;
    SmeTableImpl()
    {
      table.reserve(2000);
    }
    void Load(const char*);

    void addSme(const smsc::smeman::SmeInfo& info)
    {
      table.push_back(info);
    }
    virtual void deleteSme(const smsc::smeman::SmeSystemId& systemid)
    {
      smsc::smeman::SmeIndex idx=lookup(systemid);
      if(idx!=smsc::smeman::INVALID_SME_INDEX)
      {
        table[idx].disabled=true;
      }
    }
    virtual smsc::smeman::SmeIterator* iterator(){return 0;}
    virtual void updateSmeInfo(const smsc::smeman::SmeSystemId& systemid,const smsc::smeman::SmeInfo& newinfo)
    {
      smsc::smeman::SmeIndex idx=lookup(systemid);
      if(idx!=smsc::smeman::INVALID_SME_INDEX)
      {
        table[idx]=newinfo;
      }
    }


    smsc::smeman::SmeIndex lookup(const smsc::smeman::SmeSystemId& systemId)const
    {
      for(size_t i=0;i<table.size();i++)
      {
        if(!table[i].disabled && table[i].systemId==systemId)
        {
          return (smsc::smeman::SmeIndex)i;
        }
      }
      return smsc::smeman::INVALID_SME_INDEX;
    }
    smsc::smeman::SmeProxy* getSmeProxy(smsc::smeman::SmeIndex index) const
    {
      return 0;
    }
    virtual smsc::smeman::SmeInfo getSmeInfo(smsc::smeman::SmeIndex index) const
    {
      return table[index];
    }

  };

  static SmeTableImpl* smeTable;

public:
  static void Init(const char* routeConfigPath,const char* smeConfigPath);
  static void Reload();
  static RouterRef getInstance()
  {
    return instance;
  }
  static smsc::smeman::SmeAdministrator* getSmeAdm()
  {
    return smeTable;
  }
  static smsc::smeman::SmeTable* getSmeTable()
  {
    return smeTable;
  }
};

}
}
}

#endif /* ROUTERCONFIG_HPP_ */
