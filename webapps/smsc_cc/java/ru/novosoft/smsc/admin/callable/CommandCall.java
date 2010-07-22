/*
 * Created by igork
 * Date: Mar 29, 2002
 * Time: 6:57:47 PM
 */
package ru.novosoft.smsc.admin.callable;

import org.w3c.dom.Element;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.util.*;


class CommandCall extends ServiceCommand {
  public CommandCall(String serviceId, String componentName, String methodName, Type returnType, Map args) {
    super("call", serviceId);
    Element callElem = document.getDocumentElement();
    callElem.setAttribute("component", StringEncoderDecoder.encode(componentName));
    callElem.setAttribute("method", StringEncoderDecoder.encode(methodName));
    callElem.setAttribute("returnType", returnType.getName());

    for (Object o : args.keySet()) {
      String paramName = (String) o;
      Object param = args.get(paramName);
      Element paramElem = document.createElement("param");
      callElem.appendChild(paramElem);
      paramElem.setAttribute("name", StringEncoderDecoder.encode(paramName));
      if (param == null) {
        paramElem.setAttribute("type", Type.Types[Type.StringType].getName());
        paramElem.appendChild(document.createTextNode(""));
      } else if (param instanceof String) {
        paramElem.setAttribute("type", Type.Types[Type.StringType].getName());
        paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode((String) param)));
      } else if (param instanceof Integer) {
        paramElem.setAttribute("type", Type.Types[Type.IntType].getName());
        paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Integer) param).longValue()))));
      } else if (param instanceof Long) {
        paramElem.setAttribute("type", Type.Types[Type.IntType].getName());
        paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Long) param).longValue()))));
      } else if (param instanceof Boolean) {
        paramElem.setAttribute("type", Type.Types[Type.BooleanType].getName());
        paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Boolean) param).booleanValue()))));
      } else if (param instanceof List) {
        paramElem.setAttribute("type", Type.Types[Type.StringListType].getName());
        paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(encodeStringList((List) param))));
      } else if (param instanceof Collection) {
        paramElem.setAttribute("type", Type.Types[Type.StringListType].getName());
        paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(encodeStringList(new ArrayList((Collection) param)))));
      }
    }
  }

  private String encodeStringList(List list) {
    final StringBuffer result = new StringBuffer();
    for (Iterator i = list.iterator(); i.hasNext();) {
      String s = (String) i.next();
      result.append(encodeComma(s)).append((i.hasNext() ? "," : ""));
    }
    return result.toString();
  }

  private String encodeComma(String value) {
    final StringBuffer result = new StringBuffer();
    for (int i = 0; i < value.length(); i++) {
      char c = value.charAt(i);
      if (c == ',' || c == '\\')
        result.append("\\");
      result.append(c);
    }
    return result.toString();
  }
}
