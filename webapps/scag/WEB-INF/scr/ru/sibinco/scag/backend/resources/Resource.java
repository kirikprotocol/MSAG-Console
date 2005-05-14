package ru.sibinco.smppgw.backend.resources;

import org.apache.log4j.Logger;
import org.w3c.dom.*;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


/**
 * Created by igork
 * Date: 19.05.2004
 * Time: 18:32:39
 */
public class Resource
{
  private final File file;
  private final Map params = new HashMap();
  private final Map sections = new HashMap();
  private String decimalDelimiter;
  private String listDelimiter;
  private Logger logger = Logger.getLogger(this.getClass());
  private String localeName;

  public Resource(File file, String localeName) throws IOException, ParserConfigurationException, SAXException
  {
    this.file = file;
    this.localeName = localeName;
    load();
  }

  private void load() throws IOException, ParserConfigurationException, SAXException
  {
    if (file != null && file.exists()) {
      Document document = Utils.parse(file.getAbsolutePath());
      loadSettings(document.getElementsByTagName("settings"));

      final NodeList resources = document.getElementsByTagName("resources");
      for (int i = 0; i < resources.getLength(); i++) {
        final Element resourceElem = (Element) resources.item(i);

        final NodeList sectionsNodeList = resourceElem.getElementsByTagName("section");
        for (int j = 0; j < sectionsNodeList.getLength(); j++) {
          ResourceSection section = new ResourceSection((Element) sectionsNodeList.item(j));
          sections.put(section.getName(), section);
        }

        final NodeList paramsNodeList = resourceElem.getElementsByTagName("param");
        for (int j = 0; j < paramsNodeList.getLength(); j++) {
          ResourceParam param = new ResourceParam((Element) paramsNodeList.item(j));
          params.put(param.getName(), param);
        }
      }
    }
  }

  private void loadSettings(NodeList settings)
  {
    for (int i = 0; i < settings.getLength(); i++) {
      Element settingsNode = (Element) settings.item(i);
      NodeList sections = settingsNode.getElementsByTagName("section");
      for (int j = 0; j < sections.getLength(); j++) {
        Element sectionElem = (Element) sections.item(j);
        if ("delimiter".equalsIgnoreCase(sectionElem.getAttribute("name"))) {
          final NodeList params = sectionElem.getElementsByTagName("param");
          for (int k = 0; k < params.getLength(); k++) {
            final Element param = (Element) params.item(k);
            final String paramName = param.getAttribute("name");
            final String paramValue = Utils.getNodeText(param);
            if ("decimal".equalsIgnoreCase(paramName))
              decimalDelimiter = paramValue;
            else if ("list".equalsIgnoreCase(paramName))
              listDelimiter = paramValue;
            else
              logger.warn("Unknown setting \"" + paramName + "\" in resource file \"" + file.getAbsolutePath() + "\", skipped.");
          }
        }
      }
    }
  }

  public String getId()
  {
    return localeName;
  }

  public Map getParams()
  {
    return params;
  }

  public Map getSections()
  {
    return sections;
  }

  public String getDecimalDelimiter()
  {
    return decimalDelimiter;
  }

  public String getListDelimiter()
  {
    return listDelimiter;
  }

  public String getLocaleName()
  {
    return localeName;
  }
}
