/*
 * Created by igork
 * Date: Feb 20, 2002
 * Time: 8:20:55 PM
 */
package ru.novosoft.smsc.admin.service.protocol;

import org.w3c.dom.*;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;

public class Command
{
  private Document document;

  public Command(Document doc)
  {
    document = doc;
  }

  public Command(String commandName)
  {
    try {
      DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
      document = builder.newDocument();
      Element elem = document.createElement("command");
      elem.setAttribute("name", commandName);
      document.appendChild(elem);
    } catch (ParserConfigurationException e) {
      e.printStackTrace();
    } catch (FactoryConfigurationError error) {
      error.printStackTrace();
    } catch (DOMException e) {
      e.printStackTrace();
    }
  }

  public String getText()
  {
    Element doc = document.getDocumentElement();
    return "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
            + "<!DOCTYPE command SYSTEM \"command.dtd\">\n\n"
            + getText(doc, "");
  }

  protected String getText(Element doc, String prefix)
  {
    String result = "";
    String newPrefix = prefix + "  ";
    String name = doc.getNodeName();

    result += prefix + "<" + name;
    NamedNodeMap attrs = doc.getAttributes();
    for (int i = 0; i < attrs.getLength(); i++) {
      Node node = attrs.item(i);
      if (node.getNodeType() == Node.ATTRIBUTE_NODE)
        result += " " + node.getNodeName() + "=\"" + node.getNodeValue() + "\"";
    }
    result += ">\n";

    NodeList list = doc.getChildNodes();
    for (int i = 0; i < list.getLength(); i++) {
      Node node = list.item(i);
      if (node.getNodeType() == Node.ELEMENT_NODE)
        result += getText(doc, newPrefix);
    }

    result += prefix + "</" + name + ">\n";
    return result;
  }
}
