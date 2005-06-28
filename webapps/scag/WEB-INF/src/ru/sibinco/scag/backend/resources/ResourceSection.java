package ru.sibinco.scag.backend.resources;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.HashMap;
import java.util.Map;


/**
 * Created by igork
 * Date: 20.05.2004
 * Time: 17:13:46
 */
public class ResourceSection
{
  private final String name;
  private final Map sections = new HashMap();
  private final Map params = new HashMap();

  public ResourceSection(Element sectionElement)
  {
    name = sectionElement.getAttribute("name");
    final NodeList sectionsNodeList = sectionElement.getElementsByTagName("section");
    for (int i = 0; i < sectionsNodeList.getLength(); i++) {
      ResourceSection section = new ResourceSection((Element) sectionsNodeList.item(i));
      sections.put(section.getName(), section);
    }

    final NodeList paramsNodeList = sectionElement.getElementsByTagName("param");
    for (int i = 0; i < paramsNodeList.getLength(); i++) {
      ResourceParam param = new ResourceParam((Element) paramsNodeList.item(i));
      params.put(param.getName(), param);
    }
  }

  public String getName()
  {
    return name;
  }

  public Map getSections()
  {
    return sections;
  }

  public Map getParams()
  {
    return params;
  }
}
