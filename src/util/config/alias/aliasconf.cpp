/*
	$Id$
*/

#include "aliasconf.h"

#include <fstream>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <util/Logger.h>
#include <memory>

namespace smsc {
namespace util {
namespace config {
namespace alias{

using namespace std;

AliasConfig::RecordIterator::RecordIterator(SRVector const &records_vector)
{
  iter = records_vector.begin();
  end = records_vector.end();
}

bool AliasConfig::RecordIterator::hasRecord()
{
  return iter != end;
}

AliasConfig::status AliasConfig::RecordIterator::fetchNext(AliasRecord *&record)
{
  if (iter != end)
  {
    record = *iter;
    iter++;
    return success;
  } else {
    return fail;
  }
}

AliasConfig::AliasConfig()
  : logger(smsc::util::Logger::getCategory("smsc.igork_tests.aliasconf.AliasConfig"))
{
}

void AliasConfig::clear()
{
	{
		for ( SRVector::iterator it = records.begin();
				  it != records.end();
					++it )
		{
			if ( *it ) delete *it;
		}
	}
	records.clear();
}

AliasConfig::status AliasConfig::putRecord(AliasRecord *record)
{
  records.push_back(record);
  return success;
}

AliasConfig::status AliasConfig::load(const char * const filename)
{
  try
  {
    DOM_Document document = reader.read(filename);
    DOM_Element elem = document.getDocumentElement();
    DOM_NodeList list = elem.getElementsByTagName("record");
    for (unsigned i=0; i<list.getLength(); i++)
    {
      DOM_Node node = list.item(i);
      DOM_NamedNodeMap attrs = node.getAttributes();
      auto_ptr<AliasRecord> record(new AliasRecord());
			record->aliasValue = 0;
			record->addrValue = 0;
      //record->addr = attrs.getNamedItem("addr").getNodeValue().transcode();
			{
				char* dta = attrs.getNamedItem("addr").getNodeValue().transcode();
				record->addrValue = new char[21]; memset(record->addrValue,0,21);
				int scaned = sscanf(dta,"tni:%d,npi:%d,val:%20s",
						 &record->addrTni,
						 &record->addrNpi,
						 &record->addrValue);
				delete dta;
				if ( scaned != 3 )
				{
					logger.warn("incorrect format of 'addr = \"%20s\"'",  dta);
					continue;
				}
			}
      //record->alias = attrs.getNameItem("alias").getNodeValue().transcode();
			{
				char* dta = attrs.getNamedItem("alias").getNodeValue().transcode();
				record->aliasValue = new char[21]; memset(record->aliasValue,0,21);
				int scaned = sscanf(dta,"tni:%d,npi:%d,val:%20s",
						 &record->aliasTni,
						 &record->aliasNpi,
						 &record->aliasValue);
				delete dta;
				if ( scaned != 3 )
				{
					logger.warn("incorrect format of 'alias =  \"%20s\"'", dta);
					continue;
				}
			}
			DOM_NodeList childs = node.getChildNodes();
      records.push_back(record.release());
    }
  } catch (DOMTreeReader::ParseException &e) {
    return fail;
  }
  return success;
}

AliasConfig::status AliasConfig::store(const char * const filename) const
{
  try {
    std::ofstream out(filename);
    out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
    out << "<!DOCTYPE aliases SYSTEM \"AliasRecords.dtd\">" << std::endl;
    out << "<aliases>" << std::endl;
    for (SRVector::const_iterator i = records.begin(); i != records.end(); i++)
    {
      out << "  <record "\
				"addr=\"tni:" << (*i)->addrTni <<
				",npi:" << (*i)->addrNpi <<
				",val:" << (*i)->addrValue << "\" "\
				"alias=\"tni:" << (*i)->aliasTni <<
				",npi:" << (*i)->addrNpi <<
				",val:" << (*i)->addrValue << "\"/>" << std::endl;
		}
    out << "</aliases>" << std::endl;
  } catch (...) {
    logger.error("Some errors on Aliases records storing.");
    return fail;
  }
  return success;
}

AliasConfig::RecordIterator AliasConfig::getRecordIterator() const
{
  return RecordIterator(records);
}

}
}
}
}

