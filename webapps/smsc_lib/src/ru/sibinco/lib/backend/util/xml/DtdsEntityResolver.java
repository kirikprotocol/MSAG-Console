/*
 * Created by igork
 * Date: Mar 12, 2002
 * Time: 11:05:23 PM
 */
package ru.sibinco.lib.backend.util.xml;

import org.apache.log4j.Logger;
import org.xml.sax.*;

import java.io.IOException;
import java.io.InputStream;


public class DtdsEntityResolver implements EntityResolver
{
  Logger logger = Logger.getLogger(this.getClass().getName());

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
    logger.debug("enter '" + this.getClass().getName() + "' resolveEntity(\"" + publicId + "\", \"" + systemId + "\")");
    if (systemId.endsWith(".dtd") || systemId.endsWith(".xsd")) {
     String filename = "dtds/" + getDtdName(systemId);         
      logger.debug("try filename " + filename);
      InputStream in = Thread.currentThread().getContextClassLoader().getResourceAsStream(filename);
      if (in != null) {
        final InputSource inputSource = new InputSource(in);
        logger.debug("exit '" + this.getClass().getName() + "' resolveEntity(\"" + publicId + "\", \"" + systemId + "\"): result " + inputSource);
        return inputSource;
      } else
        logger.warn("DTD systemId entity resolving failed");
    }
    logger.debug("exit '" + this.getClass().getName() + "' resolveEntity(\"" + publicId + "\", \"" + systemId + "\"): result null");
    return null;
  }
}
