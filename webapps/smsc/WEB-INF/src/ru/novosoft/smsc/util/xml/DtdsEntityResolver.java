/*
 * Created by igork
 * Date: Mar 12, 2002
 * Time: 11:05:23 PM
 */
package ru.novosoft.smsc.util.xml;

import org.apache.log4j.Category;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.IOException;
import java.io.InputStream;


public class DtdsEntityResolver implements EntityResolver
{
  Category logger = Category.getInstance(this.getClass());

  static private String getDtdName(String systemId)
  {
    int index = systemId.lastIndexOf('/');
    if (index < 0)
      index = systemId.lastIndexOf('\\');
    if (index >= 0)
      return systemId.substring(index + 1);
    else
      return systemId;
  }

  public InputSource resolveEntity(String publicId,
                                   String systemId)
          throws SAXException, IOException
  {
    logger.debug("resolving entity \"" + systemId + "\"");
    if (systemId.endsWith(".dtd"))
    {
      String filename = "dtds/" + getDtdName(systemId);
      logger.debug("filename: " + filename);
      InputStream in = Thread.currentThread().getContextClassLoader().getResourceAsStream(filename);
      if (in != null)
        return new InputSource(in);
      else
        logger.warn("DTD systemId entity resolving failed");
    }
    return null;
  }
}
