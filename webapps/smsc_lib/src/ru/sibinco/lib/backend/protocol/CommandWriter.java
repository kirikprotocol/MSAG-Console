package ru.sibinco.lib.backend.protocol;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import ru.sibinco.lib.SibincoException;

import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.*;


/**
 * Created by igork Date: 25.05.2004 Time: 16:15:27
 */
public class CommandWriter
{
  protected final Logger logger = Logger.getLogger(getClass());;

  private final OutputStream out;
  private final Transformer transformer;
  private final ByteArrayOutputStream outBuffer = new ByteArrayOutputStream();


  public CommandWriter(OutputStream outStream) throws SibincoException
  {
    out = outStream;
    try {
      transformer = TransformerFactory.newInstance().newTransformer();
    } catch (TransformerConfigurationException e) {
      logger.debug("Couldn't create XML Transormer", e);
      throw new SibincoException("Couldn't create XML Transormer", e);
    }
    //transformer.setOutputProperty(OutputKeys.DOCTYPE_PUBLIC, "file:///command.dtd");
    transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
    transformer.setOutputProperty(OutputKeys.INDENT, "yes");
    transformer.setOutputProperty(OutputKeys.METHOD, "xml");
    transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "no");
  }

  public void write(Command command)
      throws SibincoException, IOException
  {
    logger.debug("writing command " + command.getClass().getName() + " [" + command.getSystemId() + "]");
    final Document document = command.getDocument();
    try {
      outBuffer.reset();
      transformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, command.getSystemId());
      transformer.transform(new DOMSource(document), new StreamResult(outBuffer));
      writeLength(outBuffer.size());
      outBuffer.writeTo(out);
      logger.debug("Command:\n" + outBuffer.toString());
    } catch (TransformerException e) {
      logger.debug("Couldn't process command", e);
      throw new SibincoException("Couldn't process command", e);
    }
  }

  protected void writeLength(int length)
      throws IOException
  {
    byte len[] = new byte[4];
    for (int i = 0; i < 4; i++) {
      len[3 - i] = (byte) (length & 0xff);
      length >>= 8;
    }

    out.write(len);
  }

}
