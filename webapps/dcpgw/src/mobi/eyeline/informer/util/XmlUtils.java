package mobi.eyeline.informer.util;

/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:07:12 PM
 */

import mobi.eyeline.informer.util.xml.DtdsEntityResolver;
import org.w3c.dom.*;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

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

  private static final DocumentBuilderFactory documentBuilderFactory = null;
  private static final DtdsEntityResolver dtdsEntityResolver = null;

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

  public static Document parse(InputStream input) throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException {
    if (input == null)
      throw new NullPointerException("input stream is null");
    DocumentBuilderFactory factory = getDocumentBuilderFactory();
    DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(getDtdsEntityResolver());
    InputSource source = new InputSource(input);
    return builder.parse(source);
  }


  public static void storeConfig(OutputStream out, Document d, String encoding, String docType) throws TransformerException {
    Source source = new DOMSource(d.getDocumentElement());
    Result result = new StreamResult(out);
    Transformer xformer = TransformerFactory.newInstance().newTransformer();
    xformer.setOutputProperty(OutputKeys.INDENT, "yes");
    xformer.setOutputProperty(OutputKeys.ENCODING, encoding);
    if (docType != null) {
      xformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, docType);
    }
    xformer.transform(source, result);
  }
}
