package ru.sibinco.lib.backend.service;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 17.03.2005
 * Time: 20:15:47
 * To change this template use File | Settings | File Templates.
 */

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.HashMap;
import java.util.Map;


public class Method
{
  /**
   * type of return value
   */
  protected Type type;

  /**
   * method name
   */
  protected String name;

  public Type getType()
  {
    return type;
  }

  public String getName()
  {
    return name;
  }

  public Map getParams()
  {
    return params;
  }

  public boolean equals(Object obj)
  {
    if (obj != null && obj instanceof Method) {
      Method o = (Method) obj;
      return name.equals(o.name) && type.equals(o.type) && params.equals(o.params);
    }
    return false;
  }

  /**
   * method parameters
   */
  protected Map params = new HashMap();

  public Method(Element methodElem)
  {
    name = methodElem.getAttribute("name");
    type = Type.getInstance(methodElem.getAttribute("type"));
    NodeList list = methodElem.getElementsByTagName("param");
    for (int i = 0; i < list.getLength(); i++) {
      Element paramElem = (Element) list.item(i);
      Parameter p = new Parameter(paramElem);
      params.put(p.getName(), p);
    }
  }
}
