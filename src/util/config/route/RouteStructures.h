#ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
#define SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES

#include <vector>
#include <util/cstrings.h>
#include <core/buffers/Hash.hpp>

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

class smsc::util::config::route::RouteConfig;

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
  int serviceId;


  friend class smsc::util::config::route::RouteConfig;

public:
  Route()
    : id(), priority(0), sources(), destinations(), billing(false), archiving(false), enabling(true),suppressDeliveryReports(false), serviceId(0)
  {}
  Route(const Route &r)
    : id(r.id), priority(r.priority), sources(r.sources), destinations(r.destinations),
      billing(r.billing), archiving(r.archiving), enabling(r.enabling),suppressDeliveryReports(r.suppressDeliveryReports), serviceId(r.serviceId)
  {}
  Route(const std::string & rid, const int prior, bool isBilling, bool isArchiving, bool isEnabling, bool isSuppressDR,int _serviceId)
    : id(rid), priority(prior), sources(), destinations(),
      billing(isBilling), archiving(isArchiving), enabling(isEnabling), suppressDeliveryReports(isSuppressDR),serviceId(_serviceId)
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
  const char * const getId() const {return id.c_str();}
  int getPriority() {return priority;}
  const std::string & getIdString() const {return id;}
  int getServiceId() const {return serviceId;}
};
//typedef std::vector<Route> RouteVector;
typedef std::vector<Route*> RoutePVector;


}
}
}
}

#endif //ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
