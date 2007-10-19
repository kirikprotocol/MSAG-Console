#ifndef SCAG_CONFIG_ROUTESTRUCTURES
#define SCAG_CONFIG_ROUTESTRUCTURES

#include <vector>
#include <util/cstrings.h>
#include <core/buffers/Hash.hpp>
#include <sms/sms_const.h>
#include "scag/transport/smpp/router/route_types.h"

namespace scag {
namespace config {

using smsc::util::cStringCopy;
using smsc::core::buffers::Hash;
using scag::transport::smpp::router::RouteInfo;
namespace SlicingType=scag::transport::smpp::router::SlicingType;
namespace SlicingRespPolicy=scag::transport::smpp::router::SlicingRespPolicy;

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

  SourceHash sources;
  DestinationHash destinations;

  bool archiving;
  bool enabling;
  bool active;
  std::string srcSmeSystemId;
  uint8_t slicing, slicingRespPolicy;

  int serviceId;

  friend class scag::config::RouteConfig;

public:
  Route()
    : id(), sources(), destinations(), archiving(false), enabling(true), active(false), srcSmeSystemId(), serviceId(-1), 
        slicing(SlicingType::NONE)
  {
  }
  Route(const Route &r)
    : id(r.id), sources(r.sources), destinations(r.destinations),
    archiving(r.archiving), enabling(r.enabling), active(r.active),
    srcSmeSystemId(r.srcSmeSystemId), serviceId(r.serviceId), slicing(r.slicing), slicingRespPolicy(r.slicingRespPolicy)
  {}
  Route(std::string routeId, bool archiving_, bool enabling_, bool active_, std::string srcSmeSystemId_, int32_t serviceId_,
    std::string slicing_, std::string slicingRespPolicy_)
    : id(routeId), sources(), destinations(),
    archiving(archiving_), enabling(enabling_), active(active_), srcSmeSystemId(srcSmeSystemId_), serviceId(serviceId_)
  {
      slicing = !strcmp(slicing_.c_str(), "SAR") ? SlicingType::SAR : (!strcmp(slicing_.c_str(), "UDH") ? SlicingType::UDH : SlicingType::NONE);
      slicingRespPolicy = !strcmp(slicingRespPolicy_.c_str(), "ALL") ? SlicingRespPolicy::ALL : SlicingRespPolicy::ANY;
  }

  ~Route()
  {
    sources.Empty();
    destinations.Empty();
  }

  const SourceHash &getSources() const {return sources;}
  const DestinationHash &getDestinations() const {return destinations;}
  const bool isArchiving() const {return archiving;}
  const bool isEnabling() const {return enabling;}
  const bool isActive() const {return active;}
  uint8_t getSlicingType() const {return slicing;}
  uint8_t getSlicingRespPolicy() const {return slicingRespPolicy;}
  const char * const getId() const {return id.c_str();}
  //int getPriority() {return priority;}
  const std::string & getIdString() const {return id;}
  const std::string & getSrcSmeSystemId() const { return srcSmeSystemId; }
  const int getServiceId() const {return this->serviceId;}
  //const bool isTransit(){return this->transit;}
};
//typedef std::vector<Route> RouteVector;
typedef std::vector<Route*> RoutePVector;


}
}

#endif //ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
