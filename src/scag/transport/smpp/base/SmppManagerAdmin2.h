#ifndef __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPMANAGERADMIN2_H__

#include <string>
#include <list>
#include "SmppEntityInfo.h"

namespace scag2 {
namespace transport {
namespace smpp {

typedef std::list<SmppEntityAdminInfo> SmppEntityAdminInfoList;

struct SmppManagerAdmin{
  virtual void addSmppEntity(const SmppEntityInfo& info)=0;
  virtual void updateSmppEntity(const SmppEntityInfo& info)=0;
  virtual void disconnectSmppEntity(const char* sysId)=0;
  virtual void deleteSmppEntity(const char* sysId)=0;
  virtual void ReloadRoutes()=0;
  virtual SmppEntityAdminInfoList * getEntityAdminInfoList(SmppEntityType entType) = 0;

  virtual bool LoadEntityFromConfig(SmppEntityInfo& info,const char* sysId,SmppEntityType et)=0;
  virtual bool LoadMetaEntityFromConfig(MetaEntityInfo& info,const char* sysId)=0;

  virtual void addMetaEntity(MetaEntityInfo info)=0;
  virtual void updateMetaEntity(MetaEntityInfo info)=0;
  virtual void deleteMetaEntity(const char* id)=0;

  virtual void addMetaEndPoint(const char* metaId,const char* sysId)=0;
  virtual void removeMetaEndPoint(const char* metaId,const char* sysId)=0;
};

}//smpp
}//transport
}//scag


#endif
