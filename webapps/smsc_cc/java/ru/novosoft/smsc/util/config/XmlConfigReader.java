package ru.novosoft.smsc.util.config;


import java.io.IOException;
import java.io.InputStream;
import javax.xml.parsers.ParserConfigurationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.XmlUtils;

/**
 * @author Aleksandr Khalitov
 */
class XmlConfigReader {
  
  static void loadConfig(InputStream is, XmlConfig config) throws ConfigException {
    try {
      final Document doc = XmlUtils.parse(is);
      
      config.setEncoding(doc.getXmlEncoding());
      
      final NodeList configs = doc.getElementsByTagName("config");
      if (configs == null || configs.getLength() == 0)
        throw new ConfigException("Invalid config file");
      
      readXmlConfigSection(config.getEncoding(), (Element)configs.item(0), config);
    } catch (ParserConfigurationException ex) {
      throw new ConfigException(ex);
    } catch (SAXException e) {
      throw new ConfigException(e);
    } catch (IOException e) {
      throw new ConfigException(e);
    }
  }
  
  private static String getNodeText(final Node node) {
    StringBuilder result = new StringBuilder(10);
    NodeList list = node.getChildNodes();
    for (int i = 0; i < list.getLength(); i++) {
      if (list.item(i).getNodeType() == Node.TEXT_NODE)
        result.append(list.item(i).getNodeValue());
    }
    return result.toString();
  }
  
  private static String getAttributeValue(String encoding, String attValue) {
    return (encoding != null) ? attValue : StringEncoderDecoder.unicodeToString(attValue);
  }
  
  private static void readXmlConfigSection(String encoding, Element el, XmlConfigSection sec) {
    
    final NodeList childNodes = el.getChildNodes();
    Node childNode;
    Element childEl;
    XmlConfigParam confParam;
    XmlConfigSection confSection;
    for (int i=0; i<childNodes.getLength(); i++) {
      childNode = childNodes.item(i);
      if (childNode.getNodeName() == null)
        continue;
      
      if (childNode.getNodeName().equals("param")) {
        childEl = (Element)childNode;
        confParam = new XmlConfigParam(getAttributeValue(encoding, childEl.getAttribute("name")), StringEncoderDecoder.unicodeToString(getNodeText(childEl)));
        // Fill param attributes
        final NamedNodeMap atts =childEl.getAttributes();
        for (int j=0; j<atts.getLength();j++) {
          Node attr = atts.item(j);
          confParam.setAttribute(attr.getNodeName(), attr.getNodeValue());
        }
        sec.addParam(confParam);
        
      } else if (childNode.getNodeName().equals("section")) {
        childEl = (Element)childNode;
        confSection = new XmlConfigSection(getAttributeValue(encoding, childEl.getAttribute("name")));
        sec.addSection(confSection);
        readXmlConfigSection(encoding, childEl, confSection);
      }
    }
  }
}
