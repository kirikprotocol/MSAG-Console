package mobi.eyeline.informer.util.xml;

import org.apache.log4j.Logger;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.IOException;
import java.io.InputStream;


public class DtdsEntityResolver implements EntityResolver {

  private final static Logger logger = Logger.getLogger(DtdsEntityResolver.class);

  private static String getDtdName(String systemId) {
    int index = systemId.lastIndexOf('/');
    if (index < 0)
      index = systemId.lastIndexOf('\\');
    if (index >= 0)
      return systemId.substring(index + 1);
    else
      return systemId;
  }

  public InputSource resolveEntity(String publicId, String systemId) throws SAXException, IOException {
    if (logger.isDebugEnabled())
      logger.debug("resolving entity \"" + systemId + "\"");

    if (systemId.endsWith(".dtd")) {
      String filename = "dtds/" + getDtdName(systemId);
      if (logger.isDebugEnabled())
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
