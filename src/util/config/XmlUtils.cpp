#include "XmlUtils.h"
#include <xercesc/dom/DOM_NodeList.hpp>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>

DOM_Node getElementChildByTagName(const DOM_Element &node, const char * const name)
{
	if (!node.isNull())
	{
		DOM_NodeList list = node.getElementsByTagName(name);
		return list.item(0);
	} else
		return node;
}

char * getNodeAttributeByName(const DOM_Node &node, const char * const name)
{
	DOM_NamedNodeMap nodemap = node.getAttributes();
	if (nodemap.getLength() > 0)
	{
		DOM_Node attribute = nodemap.getNamedItem(name);
		if (!attribute.isNull())
		{
			return attribute.getNodeValue().transcode();
		}
	}
	return 0;
}

