#include <stdio.h>

#include <util/xml/init.h>
#include <util/xml/utilFunctions.h>
#include <util/xml/DOMTreeReader.h>

#include <logger/Logger.h>

#include "RegionsConfig.hpp"

namespace smsc {
namespace util {
namespace config {
namespace region {

RegionsConfig::~RegionsConfig()
{
  for (Regions_t::iterator iter = _regions.begin(), end_iter = _regions.end(); iter != end_iter; ++iter)
    delete iter->second;
}

const Region&
RegionsConfig::getRegion(const std::string& regionId) const
{
  Regions_t::const_iterator iter = _regions.find(regionId.c_str());
  if ( iter != _regions.end() )
    return *(iter->second);
  else
    return _nullRegion;
}

const RegionDefault&
RegionsConfig::getDefaultRegion() const
{
  return _defaultRegion;
}

RegionsConfig::RegionsIterator
RegionsConfig::getIterator() {
  return RegionsIterator(_regions.begin(), _regions.end());
}

RegionsConfig::status RegionsConfig::RegionsIterator::fetchNext(Region *&record)
{
  if (hasRecord()) {
    record = _iter->second;
    ++_iter;
    return success;
  } else
    return fail;
}

RegionsConfig::status
RegionsConfig::loadSubjects(xercesc::DOMNodeList *subjects_elements, Region::subjects_ids_lst_t& subjects_ids)
{
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");
  size_t listLength = subjects_elements->getLength();
  if ( listLength != 1 ) {
    smsc_log_error(logger, "multiple subjects elements");
    return fail;
  }
  xercesc::DOMElement *subjects_element = (xercesc::DOMElement *)subjects_elements->item(0);
  xercesc::DOMNodeList *subjects_list = subjects_element->getElementsByTagName(smsc::util::xml::XmlStr("subject"));
  size_t subjectsListLength = subjects_list->getLength();
  for (size_t i=0; i<subjectsListLength; i++)
  {
    xercesc::DOMElement *subject_element = (xercesc::DOMElement *)subjects_list->item(i);
    smsc::util::xml::XmlStr subject_id(subject_element->getAttribute(smsc::util::xml::XmlStr("id")));
    subjects_ids.push_back(std::string(subject_id));
    smsc_log_info(logger, "subject id=%s", (const char  *)subject_id);
  }

  return success;
}

RegionsConfig::status
RegionsConfig::load()
{
  smsc::util::xml::initXerces();
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");
  try
  {
    smsc::util::xml::DOMTreeReader reader;
    smsc_log_info(logger, "DOMTreeReader.read(%s)", _filename.c_str());
    xercesc::DOMDocument *document = reader.read(_filename.c_str());
    xercesc::DOMElement *root_elem;
    smsc_log_info(logger, "try get root element");
    if (document && (root_elem=document->getDocumentElement()))
    {
      xercesc::DOMElement *elem;
      xercesc::DOMNodeList *region_defs = root_elem->getElementsByTagName(smsc::util::xml::XmlStr("region"));
      size_t listLength = region_defs->getLength();
      for (size_t i=0; i<listLength; i++)
      {
        xercesc::DOMElement *region_element = (xercesc::DOMElement *)region_defs->item(i);

        smsc::util::xml::XmlStr region_id(region_element->getAttribute(smsc::util::xml::XmlStr("id")));
        smsc::util::xml::XmlStr region_name(region_element->getAttribute(smsc::util::xml::XmlStr("name")));
        smsc::util::xml::XmlStr region_bandwidth(region_element->getAttribute(smsc::util::xml::XmlStr("bandwidth")));
        smsc::util::xml::XmlStr region_email(region_element->getAttribute(smsc::util::xml::XmlStr("email")));
        smsc::util::xml::XmlStr region_infosmeSmscId(region_element->getAttribute(smsc::util::xml::XmlStr("infosme_smsc")));

        smsc_log_info(logger, "RegionsConfig::load::: id=%s name=%s bandwidth=%d email=%s infosme_smsc=%s",
                       (const char *)region_id, (const char *)region_name, atol(region_bandwidth),(const char*)region_email, (const char*)region_infosmeSmscId);

        Region::subjects_ids_lst_t subjects_ids;
        if ( loadSubjects(region_element->getElementsByTagName(smsc::util::xml::XmlStr("subjects")), subjects_ids) != success ) return fail;
        _regions.insert(std::make_pair(std::string(region_id), new Region((const char *)region_id,
                                                                          (const char *)region_name,
                                                                          atol(region_bandwidth),
                                                                          region_email.c_str(),
                                                                          subjects_ids,
                                                                          region_infosmeSmscId.c_str())));
      }

      // load default region
      region_defs = root_elem->getElementsByTagName(smsc::util::xml::XmlStr("region_default"));
      listLength = region_defs->getLength();
      if ( listLength != 1 ) {
        smsc_log_error(logger, "RegionsConfig::load::: exact one <region_default> xml-element must exist");
        return fail;
      }

      xercesc::DOMElement *default_region_element = (xercesc::DOMElement *)region_defs->item(0);

      smsc::util::xml::XmlStr default_region_bandwidth(default_region_element->getAttribute(smsc::util::xml::XmlStr("bandwidth")));
      smsc::util::xml::XmlStr default_region_email(default_region_element->getAttribute(smsc::util::xml::XmlStr("email")));


      smsc_log_info(logger, "RegionsConfig::load::: load default region with bandwidth=%d", atol(default_region_bandwidth),default_region_email.c_str());
      _defaultRegion = RegionDefault(atol(default_region_bandwidth),default_region_email.c_str());
    } else {
      smsc_log_error(logger, "RegionsConfig::load::: Parse result is null");
      return fail;
    }

    return success;
  } catch (std::exception& ex) {
    smsc_log_info(logger, "RegionsConfig::load::: catch exception=[%s]", ex.what());
    return fail;
  }
}

}}}}
