package ru.novosoft.smsc.util.config;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.XmlUtils;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;
//import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

/**
 * @author Aleksandr Khalitov
 */

class XmlConfigWriter {

  private static DocumentBuilder docBuilder;

  static {
    try{
      DocumentBuilderFactory dbfac = DocumentBuilderFactory.newInstance();
      docBuilder = dbfac.newDocumentBuilder();
    }catch (Exception e){
      e.printStackTrace();
      throw new RuntimeException(e);
    }
  }
//
//  public static void main(String[] args) throws Exception{
//    XmlConfig c = new XmlConfig();
//    c.addParam(new XmlConfigParam("test", "testValue"));
//    XmlConfigSection s = new XmlConfigSection("testS");
//    s.setString("test", "testStringValue");
//    c.addSection(s);
//    OutputStream os = null;
//    try{
//      os = new FileOutputStream("file.test");
//      XmlConfigWriter.writeConfig(c, os, c.getEncoding() == null ? "utf-8" : c.getEncoding(), "configuration.dtd");
//    }finally {
//      if(os != null) {
//        try{
//          os.close();
//        }catch (IOException e){}
//      }
//    }
//  }

  static void writeConfig(final XmlConfig config, final OutputStream os, String encoding, String docType) throws IOException, ParserConfigurationException, TransformerException {
    Document doc = createDocument(config);
    XmlUtils.storeConfig(os, doc, encoding, docType);
  }

  private static Document createDocument(final XmlConfig config) throws ParserConfigurationException {
    Document doc = docBuilder.newDocument();

    Element root = doc.createElement("config");
    doc.appendChild(root);

    for(XmlConfigParam p : config.params()) {
      addParameter(doc, root, p);
    }

    for(XmlConfigSection s : config.sections()) {
      addSection(doc, root, s);
    }

    return doc;
  }

  private static void addSection(Document doc, Element parent, XmlConfigSection s) {
    Element sElement = doc.createElement("section");
    sElement.setAttribute("name", s.getName());
    for(XmlConfigParam param: s.params()) {
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
