package ru.sibinco.lib.backend.protocol;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;


/**
 * Created by igork
 * Date: 25.05.2004
 * Time: 16:15:59
 */
public class Command
{
  protected final Document document;
  protected final Logger logger = Logger.getLogger(this.getClass());
  private final String systemId;

  public Command(String commandName, String systemId) throws SibincoException
  {
    try {
      this.systemId = systemId;
      DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
      document = builder.newDocument();
      Element elem = document.createElement("command");
      elem.setAttribute("name", StringEncoderDecoder.encode(commandName));
      document.appendChild(elem);
    } catch (Throwable e) {
      logger.error("Couldn't create command \"" + commandName + "\"", e);
      throw new SibincoException("Couldn't create command \"" + commandName + "\"", e);
    }
  }

  protected Command(String commandName) throws SibincoException
  {
    this(commandName, "file:///command.dtd");
  }

  public Document getDocument()
  {
    return document;
  }

  public String getSystemId()
  {
    return systemId;
  }
}
