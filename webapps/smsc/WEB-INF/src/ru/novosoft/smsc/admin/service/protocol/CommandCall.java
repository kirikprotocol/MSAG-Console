/*
 * Created by igork
 * Date: Mar 29, 2002
 * Time: 6:57:47 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;

import java.util.*;

import ru.novosoft.smsc.admin.service.Type;

public class CommandCall extends ServiceCommand
{
  public CommandCall(String serviceName, String componentName, String methodName, Type returnType, Map args)
  {
    super("call", serviceName);
    Element callElem = document.getDocumentElement();
    callElem.setAttribute("component", componentName);
    callElem.setAttribute("method", methodName);
    callElem.setAttribute("returnType", returnType.getName());

    for (Iterator i = args.keySet().iterator(); i.hasNext();) {
      String paramName = (String) i.next();
      Object param = args.get(paramName);
      Element paramElem = document.createElement("param");
      callElem.appendChild(paramElem);
      paramElem.setAttribute("name", paramName);
      if (param instanceof String) {
        paramElem.setAttribute("type", "string");
        paramElem.appendChild(document.createTextNode((String) param));
      } else if (param instanceof Integer) {
        paramElem.setAttribute("type", "int");
        paramElem.appendChild(document.createTextNode(String.valueOf(((Integer) param).longValue())));
      } else if (param instanceof Long) {
        paramElem.setAttribute("type", "int");
        paramElem.appendChild(document.createTextNode(String.valueOf(((Long) param).longValue())));
      } else if (param instanceof Boolean) {
        paramElem.setAttribute("type", "bool");
        paramElem.appendChild(document.createTextNode(String.valueOf(((Boolean) param).booleanValue())));
      }
    }
  }
}
