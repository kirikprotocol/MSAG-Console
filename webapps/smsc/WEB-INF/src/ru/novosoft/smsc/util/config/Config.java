/*
* Created by igork
* Date: Mar 12, 2002
* Time: 2:56:46 PM
*/
package ru.novosoft.smsc.util.config;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.*;

import ru.novosoft.smsc.util.xml.Utils;

public class Config
{
  protected Map params = new HashMap();

  public class ParamNotFoundException extends Exception
  {
    public ParamNotFoundException(String s)
    {
      super(s);
    }
  }

  public class WrongParamTypeException extends Exception
  {
    public WrongParamTypeException(String s)
    {
      super(s);
    }
  }

  public Config(Document doc)
  {
    parseNode("", doc.getDocumentElement());
  }

  protected void parseNode(final String prefix, final Element elem)
  {
    String fullName = prefix == null || prefix.equals("")
            ? elem.getAttribute("name")
            : prefix + "." + elem.getAttribute("name");

    NodeList list = elem.getElementsByTagName("section");
    for (int i = 0; i < list.getLength(); i++) {
      parseNode(fullName, (Element) list.item(i));
    }

    list = elem.getElementsByTagName("param");
    for (int i = 0; i < list.getLength(); i++) {
      parseParamNode(fullName, (Element) list.item(i));
    }
  }

  protected void parseParamNode(final String prefix, final Element elem)
  {
    String fullName = prefix == null || prefix.equals("")
            ? elem.getAttribute("name")
            : prefix + "." + elem.getAttribute("name");
    String type = elem.getAttribute("type");
    String value = Utils.getNodeText(elem);
    if (type.equalsIgnoreCase("int")) {
      params.put(fullName, new Integer(value));
    } else if (type.equalsIgnoreCase("boolean")) {
      params.put(fullName, new Boolean(value));
    } else {
      params.put(fullName, value);
    }
  }

  public int getInt(String paramName)
          throws ParamNotFoundException, WrongParamTypeException
  {
    Object value = params.get(paramName);
    if (value == null)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof Integer)
      return ((Integer) value).intValue();
    else
      throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not integer");
  }

  public String getString(String paramName)
          throws ParamNotFoundException, WrongParamTypeException
  {
    Object value = params.get(paramName);
    if (value == null)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof String)
      return (String) value;
    else
      throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not string");
  }

  public boolean getBool(String paramName)
          throws ParamNotFoundException, WrongParamTypeException
  {
    Object value = params.get(paramName);
    if (value == null)
      throw new ParamNotFoundException("Parameter \"" + paramName + "\" not found");
    if (value instanceof Boolean)
      return ((Boolean) value).booleanValue();
    else
      throw new WrongParamTypeException("Parameter \"" + paramName + "\" is not boolean");
  }

  public Object getParameter(String paramName)
  {
    return params.get(paramName);
  }

  public Set getParameterNames()
  {
    return params.keySet();
  }

  public void setInt(String paramName, int value)
  {
    params.put(paramName, new Integer(value));
  }

  public void setString(String paramName, String value)
  {
    params.put(paramName, value);
  }

  public void setBool(String paramName, boolean value)
  {
    params.put(paramName, new Boolean(value));
  }
}
