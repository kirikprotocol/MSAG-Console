#include "utilFunctions.h"

#include <xercesc/dom/DOM_Node.hpp>
#include <xercesc/dom/DOM_NodeList.hpp>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <string>
#include <util/cstrings.h>

namespace smsc {
namespace util {
namespace xml {

using smsc::util::cStringCopy;

char * getNodeText(DOM_Node node)
{
	if (!node.isNull())
	{
		std::string s;

		DOM_NodeList childs = node.getChildNodes();
		for (int i=0; i<childs.getLength(); i++)
		{
			DOM_Node child = childs.item(i);
			if (child.getNodeType() == DOM_Node::TEXT_NODE)
			{
				char * text = child.getNodeValue().transcode();
				s += text;
				delete[] text;
			}
		}
		return cStringCopy(s.c_str());
	} else
		return 0;
}

char * getNodeAttribute(DOM_Node node, const char * const attrName)
{
	if (!node.isNull())
	{
		DOM_NamedNodeMap attrs = node.getAttributes();
		DOM_Node a = attrs.getNamedItem(attrName);
		if (!a.isNull())
		{
			return a.getNodeValue().transcode();
		}
	}
	
	return 0;
}

}
}
}
