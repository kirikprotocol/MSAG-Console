/*
 * Created by igork
 * Date: Mar 29, 2002
 * Time: 5:39:23 PM
 */
package ru.novosoft.smsc.admin.service;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.*;

import ru.novosoft.smsc.util.StringEncoderDecoder;

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
    this.name = StringEncoderDecoder.decode(elem.getAttribute("name"));
    NodeList list = elem.getElementsByTagName("method");
    for (int i = 0; i < list.getLength(); i++) {
      Element methodElem = (Element) list.item(i);
      Method newMethod = new Method(methodElem);
      methods.put(newMethod.getName(), newMethod);
    }
  }
}
