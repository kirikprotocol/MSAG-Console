#ifndef SMSC_ADMIN_SERVICE_METHOD
#define SMSC_ADMIN_SERVICE_METHOD

#include <xercesc/dom/DOM_Node.hpp>
#include <xercesc/dom/DOM_NodeList.hpp>
#include <xercesc/dom/DOM_Element.hpp>
#include <admin/service/Parameter.h>
#include <admin/service/Type.h>
#include <admin/service/Variant.h>
#include <core/buffers/Hash.hpp>
#include <util/cstrings.h>
#include <util/xml/utilFunctions.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::core::buffers::Hash;
using smsc::util::cStringCopy;
using smsc::util::xml::getNodeAttribute;
using smsc::util::xml::getNodeText;
using smsc::util::decode;

class Method
{
private:
	char * name;
	Parameters params;
	Type returnType;
	unsigned id;

public:
	Method(unsigned methodId,
				 const char * const methodName,
				 Parameters &methodParameters,
				 Type methodReturnType)
	{
		init(methodId, methodName, methodParameters , methodReturnType);
	}

	Method(DOM_Element methodElem)
		throw (AdminException)
	{
		name = decode(methodElem.getAttribute("method"));
		std::auto_ptr<char> methodReturnTypeString(methodElem.getAttribute("returnType").transcode());
		returnType = cstr2Type(methodReturnTypeString.get());
		id = 0;
		DOM_NodeList childs = methodElem.getElementsByTagName("param");
		for (unsigned i=0; i<childs.getLength(); i++)
		{
			DOM_Node paramNode = childs.item(i);
			DOM_Element &elem = (DOM_Element&) paramNode;
			std::auto_ptr<char> paramName(decode(elem.getAttribute("name")));
			std::auto_ptr<char> paramtypeStr(decode(elem.getAttribute("type")));
			params[paramName.get()]
				= Parameter(paramName.get(), cstr2Type(paramtypeStr.get()));
		}
	}

	Method(Method &copy)
	{
		init(copy.id, copy.name, copy.params, copy.returnType);
	}

	Method()
	{
		Parameters parameters;
		init(0, 0, parameters, undefined);
	}

	Method & operator = (const Method & copy)
	{
		uninit();
		init(copy.id, copy.name, copy.params, copy.returnType);
		return *this;
	}

	virtual ~Method()
	{
		uninit();
	}

	const unsigned getId() const
	{
		return id;
	}

	const char * const getName() const
	{
		return name;
	}

	const Parameters & getParameters() const
	{
		return params;
	}

	const Type getReturnType() const
	{
		return returnType;
	}

	bool operator == (const Method &method) const
	{
		bool paramIsEqual = true;
		char * pName;
		Parameter p;

		for (Parameters::Iterator i = params.getIterator(); i.Next(pName, p); )
		{
			paramIsEqual &= method.params.Exists(pName)
			                 && p == method.params[pName];
			if (!paramIsEqual)
				break;
		}

		for (Parameters::Iterator i = method.params.getIterator(); i.Next(pName, p); )
		{
			paramIsEqual &= params.Exists(pName) && p == params[pName];
			if (!paramIsEqual)
				break;
		}
		
		return (paramIsEqual
						&& name != 0 && method.name != 0
						&& strcmp(name,  method.name) == 0
						&& returnType == method.returnType);
	}

	bool operator != (const Method &method) const
	{
		return !(*this == method);
	}

protected:
	void init(const unsigned methodId,
						const char * const methodName,
						const Parameters &methodParameters,
						const Type methodReturnType)
	{
		id = methodId;
		name = cStringCopy(methodName);
		char * pname;
		Parameter p;
		for (Parameters::Iterator i = methodParameters.getIterator(); i.Next(pname, p); )
		{
			params[pname] = p;
		}
		returnType = methodReturnType;
	}

	void uninit()
	{
		id = 0;
		if (name != 0)
			delete[] name;
		params.Empty();
		returnType = undefined;

	}
};

typedef Hash<Method> Methods;

}
}
}

#endif
