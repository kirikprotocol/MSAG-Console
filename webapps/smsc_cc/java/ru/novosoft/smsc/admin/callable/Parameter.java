package ru.novosoft.smsc.admin.callable;

/*
 * Created by igork
 * Date: Mar 29, 2002
 * Time: 6:40:49 PM
 */

import org.w3c.dom.Element;


class Parameter {
  protected Type type;
  protected String name;

  public Type getType() {
    return type;
  }

  public String getName() {
    return name;
  }

  public Parameter(Element paramElement) {
    name = paramElement.getAttribute("name");
    type = Type.getInstance(paramElement.getAttribute("type"));
  }

  public boolean equals(Object obj) {
    if (obj != null && obj instanceof Parameter) {
      Parameter p = (Parameter) obj;
      return p.name.equals(name) && p.type.equals(type);
    }
    return false;
  }
}
