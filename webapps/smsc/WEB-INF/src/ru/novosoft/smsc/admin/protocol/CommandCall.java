/*
 * Created by igork
 * Date: Mar 29, 2002
 * Time: 6:57:47 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;
import ru.novosoft.smsc.admin.service.Type;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.util.*;


public class CommandCall extends ServiceCommand
{
	public CommandCall(String serviceId, String componentName, String methodName, Type returnType, Map args)
	{
		super("call", serviceId);
		Element callElem = document.getDocumentElement();
		callElem.setAttribute("component", StringEncoderDecoder.encode(componentName));
		callElem.setAttribute("method", StringEncoderDecoder.encode(methodName));
		callElem.setAttribute("returnType", returnType.getName());

		for (Iterator i = args.keySet().iterator(); i.hasNext();)
		{
			String paramName = (String) i.next();
			Object param = args.get(paramName);
			Element paramElem = document.createElement("param");
			callElem.appendChild(paramElem);
			paramElem.setAttribute("name", StringEncoderDecoder.encode(paramName));
			if (param instanceof String)
			{
				paramElem.setAttribute("type", Type.Types[Type.StringType].getName());
				paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode((String) param)));
			}
			else if (param instanceof Integer)
			{
				paramElem.setAttribute("type", Type.Types[Type.IntType].getName());
				paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Integer) param).longValue()))));
			}
			else if (param instanceof Long)
			{
				paramElem.setAttribute("type", Type.Types[Type.IntType].getName());
				paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Long) param).longValue()))));
			}
			else if (param instanceof Boolean)
			{
				paramElem.setAttribute("type", Type.Types[Type.BooleanType].getName());
				paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Boolean) param).booleanValue()))));
			}
			else if (param instanceof List)
			{
				paramElem.setAttribute("type", Type.Types[Type.StringListType].getName());
				paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(encodeStringList((List)param))));
			}
		}
	}

	private String encodeStringList(List list)
	{
		String result = "";
		for (Iterator i = list.iterator(); i.hasNext();)
		{
			String s = (String) i.next();
         result += encodeComma(s) + (i.hasNext() ? "," : "");
		}
		return result;
	}

	private String encodeComma(String value)
	{
		String result = "";
		for (int i = 0; i < value.length(); i++)
		{
			char c = value.charAt(i);
			if (c == ',' || c == '\\')
				result += '\\';
			result += c;
		}
		return result;
	}
}
