/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:07:26 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;
import org.w3c.dom.Document;

import java.util.*;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

public class CommandSetServiceConfig extends Command
{
  public CommandSetServiceConfig(Config newConfig)
    throws AdminException
  {
    super("set_config");

    ConfigTree tree = new ConfigTree(newConfig);
    tree.insertIntoElement(document, document.getDocumentElement());
  }

  protected class ConfigTree
  {
    private Map sections = new HashMap();
    private Map params = new HashMap();

    private ConfigTree()
    {
    }

    protected ConfigTree(Config config)
    {
      for (Iterator i = config.getParameterNames().iterator(); i.hasNext();)
      {
        String name = (String)i.next();
        putParameter(name, config.getParameter(name));
      }
    }

    protected void insertIntoElement(Document doc, Element main)
    {
      for (Iterator i = sections.keySet().iterator(); i.hasNext(); )
      {
        String name = (String) i.next();
        Element elem = doc.createElement("section");
        elem.setAttribute("name", name);

        ConfigTree subtree = (ConfigTree) sections.get(name);
        subtree.insertIntoElement(doc, elem);
        main.appendChild(elem);
      }

      for (Iterator i = params.keySet().iterator(); i.hasNext(); )
      {
        String name = (String) i.next();
        Element elem = doc.createElement("param");
        elem.setAttribute("name", name);
        Object value = params.get(name);
        if (value instanceof String)
        {
          elem.setAttribute("type", "string");
          elem.appendChild(doc.createTextNode((String)value));
        }
        else if (value instanceof Integer)
        {
          elem.setAttribute("type", "int");
          elem.appendChild(doc.createTextNode(String.valueOf(((Integer)value).intValue())));
        }
        else if (value instanceof Long)
        {
          elem.setAttribute("type", "int");
          elem.appendChild(doc.createTextNode(String.valueOf(((Long)value).longValue())));
        }
        else if (value instanceof Boolean)
        {
          elem.setAttribute("type", "bool");
          elem.appendChild(doc.createTextNode(((Boolean)value).booleanValue() ? "true" : "false"));
        }
        main.appendChild(elem);
      }
    }

    private ConfigTree getSection(String sectionName)
    {
      int dotPos = sectionName.indexOf('.');
      String name = dotPos > -1 ? sectionName.substring(0, dotPos) : sectionName;
      ConfigTree subtree = (ConfigTree)sections.get(name);
      if (subtree == null)
        sections.put(name, subtree = new ConfigTree());
      if (dotPos > -1)
        return subtree.getSection(sectionName.substring(dotPos + 1));
      else
        return subtree;
    }

    private void putParameter(String parameterName, Object value)
    {
      int dotPos = parameterName.indexOf('.');
      if (dotPos > -1)
      {
        ConfigTree section = getSection(parameterName.substring(0, dotPos));
        section.params.put(parameterName.substring(dotPos +1), value);
      }
      else
      {
        params.put(parameterName, value);
      }
    }
  }
}
