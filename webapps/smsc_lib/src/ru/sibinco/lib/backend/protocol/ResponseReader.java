package ru.sibinco.lib.backend.protocol;

import org.apache.log4j.Logger;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.xml.DtdsEntityResolver;

import javax.xml.parsers.*;
import java.io.*;


/**
 * Created by igork
 * Date: 25.05.2004
 * Time: 16:15:36
 */
public class ResponseReader
{
  private Logger logger = Logger.getLogger(this.getClass());
  private InputStream in;

  private final DocumentBuilder builder;

  public ResponseReader(InputStream inStream) throws SibincoException
  {
    in = inStream;
    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    factory.setIgnoringComments(true);
    factory.setValidating(false);
    try {
      builder = factory.newDocumentBuilder();
    } catch (ParserConfigurationException e) {
      logger.debug("Couldn't create XML document builder", e);
      throw new SibincoException("Couldn't create XML document builder", e);
    }
    builder.setEntityResolver(new DtdsEntityResolver());
  }

  public Response read()
      throws IOException, SibincoException
  {
    byte[] buffer = readBytes();
    if (buffer == null)
      return null;

    InputSource source = new InputSource(new ByteArrayInputStream(buffer));
    try {
      return new Response(builder.parse(source));
    } catch (SAXException e) {
      logger.debug("Couldn' parse received response", e);
      throw new SibincoException("Couldn' parse received response", e);
    }
  }

  private byte[] readBytes()
      throws IOException
  {
    int length = readLength();
    byte buffer[] = new byte[length];
    for (int readed = 0; readed < length;) {
      int readedNow = 0;
      try {
        readedNow = in.read(buffer, readed, length - readed);
      } catch (IOException e) {
        logger.info("Couldn't read response", e);
        throw e;
      }
      if (readedNow == -1)
        return null;
      readed += readedNow;
    }
    logger.debug("Response:\n" + new String(buffer));
    return buffer;
  }

  protected int readLength()
      throws IOException
  {
    int length = 0;
    for (int i = 0; i < 4; i++) {
      int c = 0;
      try {
        c = in.read();
      } catch (IOException e) {
        logger.info("Couldn't read response length", e);
        throw e;
      }
      if (c == -1)
        throw new IOException("Couldn't read response length");
      length = (length << 8) + c;
    }
    return length;
  }

}
