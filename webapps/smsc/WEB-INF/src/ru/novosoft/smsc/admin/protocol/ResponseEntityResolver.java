/*
 * Created by igork
 * Date: Mar 12, 2002
 * Time: 11:05:23 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.apache.log4j.Category;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.FileInputStream;
import java.io.IOException;


public class ResponseEntityResolver implements EntityResolver
{
  Category logger = Category.getInstance(ResponseEntityResolver.class);

  public InputSource resolveEntity(String publicId,
                                   String systemId)
          throws SAXException, IOException
  {
    logger.debug("resolving entity \"" + systemId + "\"for response");
    if (systemId.endsWith("response.dtd"))
    {
      return new InputSource(new FileInputStream("/export/home/igork/cvs/smsc/config/response.dtd"));
    }
    else
      return null;
  }
}
