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
  // bool enabling;
  bool active;
  bool transit;
  bool statistics;
  std::string srcSmeSystemId;
  uint8_t slicing, slicingRespPolicy;

  int serviceId;

  friend class scag::config::RouteConfig;

public:
  Route()
    : id(), sources(), destinations(), archiving(false), /*enabling(true),*/ active(false),
    transit(false), statistics(true), srcSmeSystemId(),
    slicing(SlicingType::NONE), serviceId(-1)
  {
  }
  Route(const Route &r)
    : id(r.id), sources(r.sources), destinations(r.destinations),
    archiving(r.archiving), /*enabling(r.enabling),*/ active(r.active),
    transit(r.transit), statistics(r.statistics),
    srcSmeSystemId(r.srcSmeSystemId), slicing(r.slicing), slicingRespPolicy(r.slicingRespPolicy), serviceId(r.serviceId)
  {}
  Route(std::string routeId, bool archiving_, bool enabling_ /*acts as active*/, bool /*not used*/,
        bool istransit, bool hasStatistics, std::string srcSmeSystemId_, int32_t serviceId_,
    std::string slicing_, std::string slicingRespPolicy_)
    : id(routeId), sources(), destinations(),
    archiving(archiving_), // enabling(enabling_),
    active(enabling_), 
    transit(istransit), statistics(hasStatistics),
    srcSmeSystemId(srcSmeSystemId_), serviceId(serviceId_)
  {
    slicing = SlicingType::NONE;
    if (!strcmp(slicing_.c_str(), "NONE")) slicing = SlicingType::NONE;
    else if (!strcmp(slicing_.c_str(), "SAR")) slicing = SlicingType::SAR;
    else if (!strcmp(slicing_.c_str(), "UDH8")) slicing = SlicingType::UDH8;
    else if (!strcmp(slicing_.c_str(), "UDH16")) slicing = SlicingType::UDH16;
    slicingRespPolicy = !strcmp(slicingRespPolicy_.c_str(), "ANY") ? SlicingRespPolicy::ANY : SlicingRespPolicy::ALL;
  }

  ~Route()
  {
    sources.Empty();
    destinations.Empty();
  }

  const SourceHash &getSources() const {return sources;}
  const DestinationHash &getDestinations() const {return destinations;}
  // const bool isArchiving() const {return archiving;}
  // const bool isEnabling() const {return enabling;}
  const bool isActive() const {return active;}
  const bool isTransit() const {return transit;}
  const bool hasStatistics() const { return statistics; }
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

namespace scag2 {
namespace config {
using scag::config::Subject;
using scag::config::Source;
using scag::config::Destination;
using scag::config::Route;
}
}

#endif //ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
