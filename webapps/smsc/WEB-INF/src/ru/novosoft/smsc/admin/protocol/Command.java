/*
 * Created by igork
 * Date: Feb 20, 2002
 * Time: 8:20:55 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.apache.log4j.Category;
import org.w3c.dom.*;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;


public class Command
{
  protected Document document;
  protected Category logger = Category.getInstance(this.getClass().getName());

  protected Command(Document doc)
  {
    document = doc;
  }

  protected Command(String commandName)
  {
    try
    {
      DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
      document = builder.newDocument();
      Element elem = document.createElement("command");
      elem.setAttribute("name", StringEncoderDecoder.encode(commandName));
      document.appendChild(elem);
    }
    catch (ParserConfigurationException e)
    {
      e.printStackTrace();
    }
    catch (FactoryConfigurationError error)
    {
      error.printStackTrace();
    }
    catch (DOMException e)
    {
      e.printStackTrace();
    }
  }

  public String getText()
  {
    //logger.debug("start getText");
    Element doc = document.getDocumentElement();
    return "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
            + "<!DOCTYPE command SYSTEM \"command.dtd\">\n\n"
            + getText(doc, "");
  }

  protected String getText(Element doc, String prefix)
  {
    //logger.debug("getText(" + doc + ", " + prefix + ")");
    String result = "";
    String newPrefix = prefix + "  ";
    String name = doc.getNodeName();
    String value = Utils.getNodeText(doc);
    //logger.debug("name = " + name);

    result += prefix + "<" + name;
    NamedNodeMap attrs = doc.getAttributes();
    for (int i = 0; i < attrs.getLength(); i++)
    {
      Node node = attrs.item(i);
      if (node.getNodeType() == Node.ATTRIBUTE_NODE)
      {
        result += " " + node.getNodeName() + "=\"" + node.getNodeValue() + "\"";
        //logger.debug("[" + node.getNodeName() + '=' + node.getNodeValue());
      }
    }
    result += ">";
    if (value == null || value.equals(""))
      result += '\n';
    else
      result += value;

    NodeList list = doc.getChildNodes();
    for (int i = 0; i < list.getLength(); i++)
    {
      Node node = list.item(i);
      if (node.getNodeType() == Node.ELEMENT_NODE)
        result += getText((Element) node, newPrefix);
    }


    result += (value == null || value.equals("") ? prefix : "") + "</" + name + ">\n";
    return result;
  }
}
