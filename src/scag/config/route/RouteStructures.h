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
  const signed long providerId;
  const signed long categoryId;
  int ruleId;


  friend class scag::config::RouteConfig;

public:
  Route()
    : id(), sources(), destinations(), archiving(false), enabling(true), active(false), srcSmeSystemId(), providerId(-1),categoryId(-1),ruleId(-1)
  {}
  Route(const Route &r)
    : id(r.id), sources(r.sources), destinations(r.destinations),
    archiving(r.archiving), enabling(r.enabling), active(r.active),
    srcSmeSystemId(r.srcSmeSystemId), providerId(r.providerId), ruleId(r.ruleId),categoryId(r.categoryId)
  {}
  Route(std::string routeId, bool archiving_, bool enabling_, bool active_, std::string srcSmeSystemId_, int32_t providerId_, int32_t ruleId_, int32_t categoryId_)
    : id(routeId), sources(), destinations(),
    archiving(archiving_), enabling(enabling_), active(active_), srcSmeSystemId(srcSmeSystemId_), providerId(providerId_), ruleId(ruleId_), categoryId(categoryId_)
  {}

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
  const char * const getId() const {return id.c_str();}
  //int getPriority() {return priority;}
  const std::string & getIdString() const {return id;}
  const std::string & getSrcSmeSystemId() const { return srcSmeSystemId; }
  const signed long getProviderId() const {return this->providerId;}
  const int getRuleId() const {return this->ruleId;}
  const signed long getCategoryId()const{return this->categoryId;}
  //const bool isTransit(){return this->transit;}
};
//typedef std::vector<Route> RouteVector;
typedef std::vector<Route*> RoutePVector;


}
}

#endif //ifndef SMSC_UTIL_CONFIG_ROUTE_ROUTESTRUCTURES
