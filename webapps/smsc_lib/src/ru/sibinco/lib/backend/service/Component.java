package ru.sibinco.lib.backend.service;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 17.03.2005
 * Time: 20:09:56
 * To change this template use File | Settings | File Templates.
 */

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.HashMap;
import java.util.Map;


public class Component
{
  protected String name;
  protected Map methods = new HashMap();

  public Map getMethods()
  {
    return methods;
  }

  public String getName()
  {
    return name;
  }

  public Component(Element elem)
  {
    this.name = elem.getAttribute("name");
    NodeList list = elem.getElementsByTagName("method");
    for (int i = 0; i < list.getLength(); i++) {
      Element methodElem = (Element) list.item(i);
      Method newMethod = new Method(methodElem);
      methods.put(newMethod.getName(), newMethod);
    }
  }
}