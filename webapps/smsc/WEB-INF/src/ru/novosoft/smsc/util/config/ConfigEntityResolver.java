/*
 * Created by igork
 * Date: Apr 12, 2002
 * Time: 10:57:39 AM
 */
package ru.novosoft.smsc.util.config;

import org.xml.sax.*;

import java.io.IOException;


public class ConfigEntityResolver implements EntityResolver
{
  public InputSource resolveEntity(String publicId, String systemId)
          throws SAXException, IOException
  {
    if (systemId.endsWith("configuration.dtd"))
      return new InputSource(this.getClass().getClassLoader().getResourceAsStream("ru/novosoft/smsc/util/config/configuration.dtd"));
    else
      return null;
  }
}
