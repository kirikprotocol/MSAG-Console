/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:07:12 PM
 */
package ru.novosoft.smsc.util.xml;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.InputStream;


public class Utils
{
  static public String getNodeText(final Node node)
  {
    String result = "";
    NodeList list = node.getChildNodes();
    for (int i = 0; i < list.getLength(); i++)
    {
      if (list.item(i).getNodeType() == Node.TEXT_NODE)
        result += list.item(i).getNodeValue();
    }
    return result;
  }

  static public Document parse(InputStream inputStream)
          throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException, NullPointerException
  {
    if (inputStream == null)
      throw new NullPointerException("input stream is null");
    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(new DtdsEntityResolver());
    InputSource source = new InputSource(inputStream);
    return builder.parse(source);
  }
}
