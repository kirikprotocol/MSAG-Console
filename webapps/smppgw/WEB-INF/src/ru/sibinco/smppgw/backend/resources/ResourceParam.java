package ru.sibinco.smppgw.backend.resources;

import org.w3c.dom.Element;
import ru.sibinco.lib.backend.util.xml.Utils;


/**
 * Created by igork
 * Date: 20.05.2004
 * Time: 17:19:47
 */
public class ResourceParam
{
  private final String name;
  private final String value;

  public ResourceParam(Element resourceElement)
  {
    name = resourceElement.getAttribute("name");
    value = Utils.getNodeText(resourceElement);
  }

  public String getName()
  {
    return name;
  }

  public String getValue()
  {
    return value;
  }
}
