package ru.novosoft.smsc.util;

/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:07:12 PM
 */

import org.w3c.dom.*;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.util.xml.DtdsEntityResolver;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.*;


/**
 * Утилиты для работы с XML
 */
public class XmlUtils {

  private static DocumentBuilderFactory documentBuilderFactory = null;
  private static DtdsEntityResolver dtdsEntityResolver = null;

  private static synchronized DocumentBuilderFactory getDocumentBuilderFactory() {
    return documentBuilderFactory == null ? DocumentBuilderFactory.newInstance() : documentBuilderFactory;
  }

  public static synchronized DtdsEntityResolver getDtdsEntityResolver() {
    return dtdsEntityResolver == null ? new DtdsEntityResolver() : dtdsEntityResolver;
  }

  public static String getNodeText(final Node node) {
    String result = "";
    NodeList list = node.getChildNodes();
    for (int i = 0; i < list.getLength(); i++) {
      if (list.item(i).getNodeType() == Node.TEXT_NODE)
        result += list.item(i).getNodeValue();
    }
    return result;
  }

  public static Document parse(Reader input) throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException {
    if (input == null)
      throw new NullPointerException("input stream is null");
    DocumentBuilderFactory factory = getDocumentBuilderFactory();
    DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(getDtdsEntityResolver());
    InputSource source = new InputSource(input);
    return builder.parse(source);
  }

  public static Document parse(InputStream input) throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException {
    if (input == null)
      throw new NullPointerException("input stream is null");
    DocumentBuilderFactory factory = getDocumentBuilderFactory();
    DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(getDtdsEntityResolver());
    InputSource source = new InputSource(input);
    return builder.parse(source);
  }

  public static Document parse(String filename) throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException {
    if (filename == null)
      throw new NullPointerException("input filename is null");
    DocumentBuilderFactory factory = getDocumentBuilderFactory();
    DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(getDtdsEntityResolver());
    InputSource source = new InputSource(filename);
    return builder.parse(source);
  }

  public static Element createChildElement(Document document, Element parent, String newChildTagName) {
    Element newChild = document.createElement(newChildTagName);
    parent.appendChild(newChild);
    return newChild;
  }

  public static Text createTextChild(Document document, Element parent, String text) {
    Text textNode = document.createTextNode(text);
    parent.appendChild(textNode);
    return textNode;
  }

  public static Element appendFirstByTagName(Element parent, Element newElement) {
    NodeList securityConstraints = parent.getElementsByTagName(newElement.getTagName());
    if (securityConstraints.getLength() == 0)
      parent.appendChild(newElement);
    else
      parent.insertBefore(newElement, securityConstraints.item(0));

    return newElement;
  }

  public static PrintWriter storeConfigHeader(PrintWriter out, String docType, String dtdFile) {
    // C++ code doesn't know about other codings // System.getProperty("file.encoding");
    return storeConfigHeader(out, docType, dtdFile, null);
  }

  public static PrintWriter storeConfigHeader(PrintWriter out, String docType, String dtdFile, String encoding) {
    out.println("<?xml version=\"1.0\" encoding=\"" + (encoding == null || encoding.length() == 0 ? Functions.getLocaleEncoding() : encoding) + "\"?>");
    out.println("<!DOCTYPE " + docType + " SYSTEM \"" + dtdFile + "\">");
    out.println();
    out.println("<" + docType + ">");
    return out;
  }

  public static void storeConfig(OutputStream out, Document d, String encoding, String docType) throws TransformerException {
    Source source = new DOMSource(d.getDocumentElement());
    Result result = new StreamResult(out);
    Transformer xformer = TransformerFactory.newInstance().newTransformer();
    xformer.setOutputProperty(OutputKeys.INDENT, "yes");
    xformer.setOutputProperty(OutputKeys.ENCODING, encoding);
    if(docType != null) {
      xformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, docType);
    }
    xformer.transform(source, result);
  }

  public static PrintWriter storeConfigFooter(PrintWriter out, String docType) {
    out.println("</" + docType + ">");
    return out;
  }
}
