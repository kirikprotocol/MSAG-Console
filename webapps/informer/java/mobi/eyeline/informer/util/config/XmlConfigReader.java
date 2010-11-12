package mobi.eyeline.informer.util.config;


import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.util.XmlUtils;
import org.w3c.dom.*;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.InputStream;

/**
 * @author Aleksandr Khalitov
 */
class XmlConfigReader {
  
  static void loadConfig(InputStream is, XmlConfig config) throws XmlConfigException {
    try {
      final Document doc = XmlUtils.parse(is);
      
      config.setEncoding(doc.getXmlEncoding());
      
      final NodeList configs = doc.getElementsByTagName("config");
      if (configs == null || configs.getLength() == 0)
        throw new XmlConfigException("Invalid config file");
      
      readXmlConfigSection(config.getEncoding(), (Element)configs.item(0), config);
    } catch (ParserConfigurationException ex) {
      throw new XmlConfigException(ex);
    } catch (SAXException e) {
      throw new XmlConfigException(e);
    } catch (IOException e) {
      throw new XmlConfigException(e);
    }
  }
  
  private static String getNodeText(final Node node) {
    StringBuilder result = new StringBuilder(10);
    NodeList list = node.getChildNodes();
    for (int i = 0; i < list.getLength(); i++) {
      if (list.item(i).getNodeType() == Node.TEXT_NODE)
        result.append(StringEncoderDecoder.decode(list.item(i).getNodeValue()));
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
        String name = getAttributeValue(encoding, childEl.getAttribute("name"));
        String value = StringEncoderDecoder.unicodeToString(getNodeText(childEl));
        String typeStr = getAttributeValue(encoding, childEl.getAttribute("type"));
        XmlConfigParam.Type type = XmlConfigParam.Type.STRING;
        if (typeStr != null) {
          if (typeStr.equals("string"))
            type = XmlConfigParam.Type.STRING;
          else if (typeStr.equals("int"))
            type = XmlConfigParam.Type.INT;
          else
            type = XmlConfigParam.Type.BOOL;
        }

        confParam = new XmlConfigParam(name, value, type);
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
