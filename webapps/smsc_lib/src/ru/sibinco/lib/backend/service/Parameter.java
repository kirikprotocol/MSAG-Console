package ru.sibinco.lib.backend.service;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 17.03.2005
 * Time: 20:18:40
 * To change this template use File | Settings | File Templates.
 */
import org.w3c.dom.Element;


public class Parameter
{
  protected Type type;
  protected String name;

  public Type getType()
  {
    return type;
  }

  public String getName()
  {
    return name;
  }

  public Parameter(Element paramElement)
  {
    name = paramElement.getAttribute("name");
    type = Type.getInstance(paramElement.getAttribute("type"));
  }

  public boolean equals(Object obj)
  {
    if (obj != null && obj instanceof Parameter) {
      Parameter p = (Parameter) obj;
      return p.name.equals(name) && p.type.equals(type);
    }
    return false;
  }
}
