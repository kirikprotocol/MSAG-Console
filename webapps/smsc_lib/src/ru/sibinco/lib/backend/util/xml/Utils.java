/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:07:12 PM
 */
package ru.sibinco.lib.backend.util.xml;

import org.w3c.dom.*;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.*;
import java.io.IOException;
import java.io.Reader;


public class Utils
{
  private static DocumentBuilderFactory documentBuilderFactory = null;
  private static DtdsEntityResolver dtdsEntityResolver = null;

  private static DocumentBuilderFactory getDocumentBuilderFactory()
  {
    return documentBuilderFactory == null ? DocumentBuilderFactory.newInstance() : documentBuilderFactory;
  }

  public static DtdsEntityResolver getDtdsEntityResolver()
  {
    return dtdsEntityResolver == null ? new DtdsEntityResolver() : dtdsEntityResolver;
  }

  public static String getNodeText(final Node node)
  {
    String result = "";
    NodeList list = node.getChildNodes();
    for (int i = 0; i < list.getLength(); i++) {
      if (list.item(i).getNodeType() == Node.TEXT_NODE)
        result += list.item(i).getNodeValue();
    }
    return result;
  }

  public static Document parse(Reader input)
      throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException, NullPointerException
  {
    if (input == null)
      throw new NullPointerException("input stream is null");
    DocumentBuilderFactory factory = getDocumentBuilderFactory();
    DocumentBuilder builder = factory.newDocumentBuilder();    
    builder.setEntityResolver(getDtdsEntityResolver());
    InputSource source = new InputSource(input);
    return builder.parse(source);
  }

  public static Document parse(String filename)
      throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException, NullPointerException
  {
    if (filename == null)
      throw new NullPointerException("input filename is null");
    DocumentBuilderFactory factory = getDocumentBuilderFactory();
    DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(getDtdsEntityResolver());
    InputSource source = new InputSource(filename);
    return builder.parse(source);
  }
      public static Document parseSchema(String xsdFolder,String filename)
      throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException, NullPointerException
  {
    if (filename == null)
      throw new NullPointerException("input filename is null");
    DocumentBuilderFactory factory = getDocumentBuilderFactory();
    DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(getDtdsEntityResolver());
    InputSource source = new InputSource(filename);
    source.setSystemId(xsdFolder+"/xsd/smpp_rules.xsd");
    return builder.parse(source);
  }
  public static Element createChildElement(Document document, Element parent, String newChildTagName)
  {
    Element newChild = document.createElement(newChildTagName);
    parent.appendChild(newChild);
    return newChild;
  }

  public static Text createTextChild(Document document, Element parent, String text)
  {
    Text textNode = document.createTextNode(text);
    parent.appendChild(textNode);
    return textNode;
  }

  public static Element appendFirstByTagName(Element parent, Element newElement)
  {
    NodeList securityConstraints = parent.getElementsByTagName(newElement.getTagName());
    if (securityConstraints.getLength() == 0)
      parent.appendChild(newElement);
    else
      parent.insertBefore(newElement, securityConstraints.item(0));

    return newElement;
  }
}
