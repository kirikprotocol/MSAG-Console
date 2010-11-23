#ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
#define SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES

#include <vector>

#include "core/buffers/Hash.hpp"
#include "util/cstrings.h"
#include "sms/sms_const.h"
#include "smsc/acls/AclManager.hpp"
#include "smsc/router/route_types.h"

namespace smsc {
namespace config {
namespace route {

using smsc::util::cStringCopy;
using smsc::core::buffers::Hash;
using namespace smsc::acl;

typedef std::string Mask;
typedef std::vector<Mask> MaskVector;

class Subject {
private:
  std::string id;
  MaskVector masks;
  MaskVector subjRefs;
public:
  Subject()
    : id(), masks()
  {}

  Subject(const Mask & mask)
    : id(mask), masks(1, mask)
  {}

  Subject(const std::string &subjId, const MaskVector & subjMasks,const MaskVector& argSubjRefs)
    : id(subjId), masks(subjMasks),subjRefs(argSubjRefs)
  {}
  Subject(const Subject & subj)
    : id(subj.id), masks(subj.masks),subjRefs(subj.subjRefs)
  {}

  const std::string &getIdString() const {return id;}
  const char * const getId()       const {return id.c_str();}

  MaskVector& getMasks()
  {
    return masks;
  }
  const MaskVector& getMasks()const
  {
    return masks;
  }

  MaskVector& getSubjRefs()
  {
    return subjRefs;
  }
};

//typedef std::vector<Subject> SubjectVector;
//typedef std::map<char*, Subject> SubjectMap;
typedef Hash<Subject *> SubjectPHash;

class Source {
private:
  Subject subj;
  bool isSubj;

public:
  Source()
    : subj(), isSubj(true)
  {}

  Source(const Mask &m)
    : subj(m), isSubj(false)
  {}

  Source(const Subject &s)
    : subj(s), isSubj(true)
  {}
  Source(const Source &s)
    : subj(s.subj), isSubj(s.isSubj)
  {}

  const char * const getId() const {return subj.getId();}
  const std::string getIdString() const {return subj.getIdString();}
  const bool isSubject() const {return isSubj;}
  const MaskVector & getMasks() const {return subj.getMasks();}
};
//typedef std::vector<Source> SourceVector;
typedef Hash<Source> SourceHash;

typedef std::string SmeId;

class Destination : public Source
{
private:
  SmeId smeId;

public:
  Destination()
    : Source(), smeId()
  {}
  Destination(const Destination & dst)
    : Source(dst), smeId(dst.smeId)
  {}

  Destination(const Mask &m, const SmeId &smeid)
    : Source(m), smeId(smeid)
  {}

  Destination(const Subject &s, const SmeId &smeid)
    : Source(s), smeId(smeid)
  {}

  const char * const getSmeId() const {return smeId.c_str();}
  const std::string& getSmeIdString() const {return smeId;}
};
//typedef std::vector<Destination> DestinationVector;
typedef Hash<Destination> DestinationHash;

class RouteConfig;

class Route {
private:
  std::string id;
  int priority;
  SourceHash sources;
  DestinationHash destinations;
  uint8_t billing;
  bool archiving;
  smsc::router::TrafficMode trafMode;
  bool suppressDeliveryReports;
  bool active;
  bool hide;
  smsc::router::ReplyPath replyPath;
  int serviceId;
  std::string srcSmeSystemId;
  const uint8_t deliveryMode;
  std::string forwardTo;
  const AclIdent aclId;
  const bool forceDelivery;
  const bool allowBlocked;
  const signed long providerId;
  const signed long categoryId;
  bool transit;


  friend class smsc::config::route::RouteConfig;

public:
  Route()
    : id(), priority(0), sources(), destinations(), billing(0), archiving(false), trafMode(smsc::router::tmAll),
      suppressDeliveryReports(false), hide(true), replyPath(smsc::router::ReplyPathPass),
      serviceId(0), srcSmeSystemId(), deliveryMode(smsc::sms::SMSC_DEFAULT_MSG_MODE),
      forwardTo(), aclId((AclIdent)-1), forceDelivery(false), allowBlocked(false),
      providerId(-1),categoryId(-1),transit(false)
  {}
  Route(const std::string & rid, const int prior, uint8_t billingMode, bool isArchiving,
      smsc::router::TrafficMode argTrafMode, bool isSuppressDR, bool isActive,bool isHide,smsc::router::ReplyPath replyPath,
      int _serviceId, const std::string & _srcSmeSystemId, const uint8_t _deliveryMode,
      const std::string & _forwardTo,const AclIdent aclId,
      const bool forceDelivery, const bool allowBlocked, const signed long providerId,
      const signed long categoryId,bool transit)
    : id(rid), priority(prior), sources(), destinations(),
    billing(billingMode), archiving(isArchiving), trafMode(argTrafMode), suppressDeliveryReports(isSuppressDR),
    active(isActive), hide(isHide),replyPath(replyPath),serviceId(_serviceId),
    srcSmeSystemId(_srcSmeSystemId), deliveryMode(_deliveryMode), forwardTo(_forwardTo),
    aclId(aclId), forceDelivery(forceDelivery), allowBlocked(allowBlocked),
    providerId(providerId), categoryId(categoryId),transit(transit)
  {}

  ~Route()
  {
    sources.Empty();
    destinations.Empty();
  }

  const SourceHash &getSources() const {return sources;}
  const DestinationHash &getDestinations() const {return destinations;}
  const uint8_t getBilling() const {return billing;}
  const bool isArchiving() const {return archiving;}
  const smsc::router::TrafficMode getTrafMode() const {return trafMode;}
  const bool isSuppressDeliveryReports(){return suppressDeliveryReports;}
  const bool isActive(){return active;}
  const bool isHide()const{return hide;}
  const smsc::router::ReplyPath getReplyPath() const{return replyPath;}
  const char * const getId() const {return id.c_str();}
  int getPriority() {return priority;}
  const std::string & getIdString() const {return id;}
  int getServiceId() const {return serviceId;}
  const std::string & getSrcSmeSystemId() const { return srcSmeSystemId; }
  const uint8_t getDeliveryMode() const {return this->deliveryMode;}
  const std::string & getForwardTo() const {return this->forwardTo;}
  const AclIdent getAclId() const { return this->aclId; }
  const bool isForceDelivery() const { return this->forceDelivery; }
  const bool isAllowBlocked() const { return this->allowBlocked; }
  const signed long getProviderId() const {return this->providerId;}
  const signed long getCategoryId()const{return this->categoryId;}
  const bool isTransit(){return this->transit;}
};
//typedef std::vector<Route> RouteVector;
typedef std::vector<Route*> RoutePVector;


}
}
}

#endif //ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
