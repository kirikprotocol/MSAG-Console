#ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
#define SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES

#include <vector>
#include <util/cstrings.h>
#include <core/buffers/Hash.hpp>
#include <sms/sms_const.h>

namespace smsc {
namespace util {
namespace config {
namespace route {

using smsc::util::cStringCopy;
using smsc::core::buffers::Hash;

typedef std::string Mask;
typedef std::vector<Mask> MaskVector;

class Subject {
private:
  std::string id;
  MaskVector masks;
public:
  Subject()
    : id(), masks()
  {}
  Subject(const Mask & mask)
    : id(mask), masks(1, mask)
  {}
  Subject(const std::string &subjId, const MaskVector & subjMasks)
    : id(subjId), masks(subjMasks)
  {}
  Subject(const Subject & subj)
    : id(subj.id), masks(subj.masks)
  {}

  const std::string &getIdString() const {return id;}
  const char * const getId()       const {return id.c_str();}

  const MaskVector & getMasks()    const {return masks;}
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
  bool billing;
  bool archiving;
  bool enabling;
  bool suppressDeliveryReports;
  bool active;
  bool hide;
  bool forceReplyPath;
  int serviceId;
  std::string srcSmeSystemId;
  const uint8_t deliveryMode;
  std::string forwardTo;
  std::string trafrules;


  friend class smsc::util::config::route::RouteConfig;

public:
  Route()
    : id(), priority(0), sources(), destinations(), billing(false), archiving(false), enabling(true),suppressDeliveryReports(false), hide(true), forceReplyPath(false), serviceId(0), srcSmeSystemId(), deliveryMode(smsc::sms::SMSC_DEFAULT_MSG_MODE), forwardTo()
  {}
  Route(const Route &r)
    : id(r.id), priority(r.priority), sources(r.sources), destinations(r.destinations),
    billing(r.billing), archiving(r.archiving), enabling(r.enabling),suppressDeliveryReports(r.suppressDeliveryReports), serviceId(r.serviceId),
    active(r.active),hide(r.hide),forceReplyPath(r.forceReplyPath),
    srcSmeSystemId(r.srcSmeSystemId), deliveryMode(r.deliveryMode), forwardTo(r.forwardTo),trafrules(r.trafrules)
  {}
  Route(const std::string & rid, const int prior, bool isBilling, bool isArchiving, bool isEnabling, bool isSuppressDR, bool isActive,bool isHide,bool isForceRP, int _serviceId, const std::string & _srcSmeSystemId, const uint8_t _deliveryMode, const std::string & _forwardTo,const std::string& _trafrules)
    : id(rid), priority(prior), sources(), destinations(),
    billing(isBilling), archiving(isArchiving), enabling(isEnabling), suppressDeliveryReports(isSuppressDR), active(isActive), hide(isHide),forceReplyPath(isForceRP),serviceId(_serviceId),
    srcSmeSystemId(_srcSmeSystemId), deliveryMode(_deliveryMode), forwardTo(_forwardTo),trafrules(_trafrules)
  {}

  ~Route()
  {
    sources.Empty();
    destinations.Empty();
  }

  const SourceHash &getSources() const {return sources;}
  const DestinationHash &getDestinations() const {return destinations;}
  const bool isBilling() const {return billing;}
  const bool isArchiving() const {return archiving;}
  const bool isEnabling() const {return enabling;}
  const bool isSuppressDeliveryReports(){return suppressDeliveryReports;}
  const bool isActive(){return active;}
  const bool isHide()const{return hide;}
  const bool isForceReplyPath() const{return forceReplyPath;}
  const char * const getId() const {return id.c_str();}
  int getPriority() {return priority;}
  const std::string & getIdString() const {return id;}
  int getServiceId() const {return serviceId;}
  const std::string & getSrcSmeSystemId() const { return srcSmeSystemId; }
  const uint8_t getDeliveryMode() const {return this->deliveryMode;}
  const std::string & getForwardTo() const {return this->forwardTo;}
  const std::string& getTrafRules()const{return this->trafrules;}
};
//typedef std::vector<Route> RouteVector;
typedef std::vector<Route*> RoutePVector;


}
}
}
}

#endif //ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
