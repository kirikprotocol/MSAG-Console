package ru.sibinco.lib.backend.protocol;

import org.apache.log4j.Logger;
import org.w3c.dom.*;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import javax.xml.parsers.*;


/**
 * Created by igork
 * Date: 25.05.2004
 * Time: 16:15:59
 */
public class Command
{
  protected Document document;
  protected Logger logger = Logger.getLogger(this.getClass());

  protected Command(String commandName)
  {
    try {
      DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
      document = builder.newDocument();
      Element elem = document.createElement("command");
      elem.setAttribute("name", StringEncoderDecoder.encode(commandName));
      document.appendChild(elem);
    } catch (ParserConfigurationException e) {
      e.printStackTrace();
    } catch (FactoryConfigurationError error) {
      error.printStackTrace();
    } catch (DOMException e) {
      e.printStackTrace();
    }
  }

  public Document getDocument()
  {
    return document;
  }
}
