package mobi.eyeline.informer.util.config;

import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.util.XmlUtils;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;
import java.io.IOException;
import java.io.OutputStream;

/**
 * @author Aleksandr Khalitov
 */

class XmlConfigWriter {

  private XmlConfigWriter() {}

  private static final DocumentBuilder docBuilder;

  static {
    try {
      DocumentBuilderFactory dbfac = DocumentBuilderFactory.newInstance();
      docBuilder = dbfac.newDocumentBuilder();
    } catch (Exception e) {
      e.printStackTrace();
      throw new RuntimeException(e);
    }
  }

  static void writeConfig(final XmlConfig config, final OutputStream os, String encoding, String docType) throws IOException, ParserConfigurationException, TransformerException {
    Document doc = createDocument(config);
    XmlUtils.storeConfig(os, doc, encoding, docType);
  }

  private static Document createDocument(final XmlConfig config) throws ParserConfigurationException {
    Document doc = docBuilder.newDocument();

    Element root = doc.createElement("config");
    doc.appendChild(root);

    for (XmlConfigParam p : config.params()) {
      addParameter(doc, root, p);
    }

    for (XmlConfigSection s : config.sections()) {
      addSection(doc, root, s);
    }

    return doc;
  }

  private static void addSection(Document doc, Element parent, XmlConfigSection s) {
    Element sElement = doc.createElement("section");
    sElement.setAttribute("name", s.getName());
    for (XmlConfigParam param : s.params()) {
      addParameter(doc, sElement, param);
    }
    parent.appendChild(sElement);
    for (XmlConfigSection childSection : s.sections()) {
      addSection(doc, sElement, childSection);
    }
  }

  private static void addParameter(Document doc, Element section, XmlConfigParam p) {
    Element param = doc.createElement("param");
    param.setAttribute("name", p.getName());
    param.setAttribute("type", p.getType().toString().toLowerCase());
    param.setTextContent(StringEncoderDecoder.encode(p.getString()));
    section.appendChild(param);
  }

}
