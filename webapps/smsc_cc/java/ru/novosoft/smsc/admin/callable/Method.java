package ru.novosoft.smsc.admin.callable;

/*
 * Created by igork
 * Date: Mar 29, 2002
 * Time: 6:20:27 PM
 */

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.HashMap;
import java.util.Map;


class Method {
  /**
   * type of return value
   */
  protected Type type;

  /**
   * method name
   */
  protected String name;

  public Type getType() {
    return type;
  }

  public String getName() {
    return name;
  }

  public Map getParams() {
    return params;
  }

  public boolean equals(Object obj) {
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

  public Method(Element methodElem) {
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
