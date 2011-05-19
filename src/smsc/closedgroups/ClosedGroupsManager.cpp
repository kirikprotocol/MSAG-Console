#include <utility>

#include "util/xml/DOMTreeReader.h"
#include "util/xml/utilFunctions.h"
#include "util/Exception.hpp"

#include "ClosedGroupsManager.hpp"

namespace smsc{
namespace closedgroups{

ClosedGroupsInterface* ClosedGroupsInterface::instance;


void ClosedGroupsManager::Load(const char* filename)
{
  sync::MutexGuard mg(mtx);

  using namespace smsc::util::xml;
  DOMTreeReader reader;
  DOMDocument* doc=reader.read(filename);
  DOMElement *elem = doc->getDocumentElement();
  DOMNodeList *list = elem->getElementsByTagName(XmlStr("lastId"));
  if(list->getLength()!=1)
  {
    throw smsc::util::Exception("Wrong number of elements lastId in %s:%d",filename,list->getLength());
  }
  //lastId=AttrGetIntValue(list->item(0)->getAttributes(),"value");

  list = elem->getElementsByTagName(XmlStr("group"));
  size_t listLength = list->getLength();
  for (size_t i=0; i<listLength; i++)
  {
    DOMNode *group = list->item(i);
    std::auto_ptr<Group> grp(new Group);
    int id=AttrGetIntValue(group->getAttributes(),"id");
    grp->id=id;
    AttrFillStringValue(group->getAttributes(),"name",grp->name);
    for(DOMNode *node = group->getFirstChild(); node != 0; node = node->getNextSibling())
    {
      if(node->getNodeType()!=DOMNode::ELEMENT_NODE)continue;
      if(XmlStr(node->getNodeName())!="mask" && XmlStr(node->getNodeName())!="address")
      {
        throw smsc::util::Exception("Unknown node name in group:%s",XmlStr(node->getNodeName()).c_str());
      }
      buf::FixedLengthString<32> addr;
      AttrFillStringValue(node->getAttributes(),"value",addr);
      grp->addresses.insert(smsc::sms::Address(addr.c_str()));
    }
    groups.insert(GroupsMap::value_type(id,grp.release()));
  }

}

}
}
