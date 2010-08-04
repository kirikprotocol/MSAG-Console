package ru.novosoft.smsc.admin.resource;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.XmlUtils;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
class ResourceFile implements ManagedConfigFile<ResourceSettings> {

  private void storeResources(Resources section, PrintWriter out, String prefix) {
    for (Map.Entry<String, Resources> e : section.getChilds().entrySet()) {
      String sectionName = e.getKey();
      out.println(prefix + "<section name=\"" + StringEncoderDecoder.encode(sectionName) + "\">");
      storeResources(e.getValue(), out, prefix + "   ");
      out.println(prefix + "</section>");
    }
    for (Map.Entry<String, String> e : section.getStrings().entrySet()) {
      String paramName = e.getKey();
      String paramValue = e.getValue();
      if (paramValue != null && paramValue.length() > 0)
        out.println(prefix + "<param name=\"" + StringEncoderDecoder.encode(paramName) + "\">" + StringEncoderDecoder.encode(paramValue) + "</param>");
      else
        out.println(prefix + "<param name=\"" + StringEncoderDecoder.encode(paramName) + "\"/>");
    }
  }

  public void save(InputStream oldFile, OutputStream newFile, ResourceSettings conf) throws Exception {

    PrintWriter out = null;
    try {
      out = new PrintWriter(new OutputStreamWriter(newFile, Functions.getLocaleEncoding()));
      XmlUtils.storeConfigHeader(out, "locale_resources", "locale_resources.dtd");
      out.println("<settings>");
      out.println("  <section name=\"delimiter\">");
      out.println("    <param name=\"decimal\">" + conf.getDecimalDelimiter() + "</param>");
      out.println("    <param name=\"list\">" + conf.getListDelimiter() + "</param>");
      out.println("  </section>");
      out.println("</settings>");

      out.println("<resources>");
      storeResources(conf.getResources(), out, "   ");
      out.println("</resources>");
      XmlUtils.storeConfigFooter(out, "locale_resources");
      out.flush();
      out.close();
    } finally {
      if (out != null)
        out.close();
    }
  }

  private void loadResourceList(Element sectionElem, Resources l) {

    Map<String, Resources> childs = new HashMap<String, Resources>();
    Map<String, String> strings = new HashMap<String, String>();
    NodeList childNodes = sectionElem.getChildNodes();
    for (int i = 0; i < childNodes.getLength(); i++) {
      Node childNode = childNodes.item(i);
      if (childNode.getNodeType() == Node.ELEMENT_NODE) {
        Element childElement = (Element) childNode;
        if ("section".equalsIgnoreCase(childElement.getTagName())) {
          String subSectionName = childElement.getAttribute("name");
          Resources subSection = new Resources();
          loadResourceList(childElement, subSection);
          childs.put(subSectionName, subSection);
        } else if ("param".equalsIgnoreCase(childElement.getTagName())) {
          String paramName = childElement.getAttribute("name");
          String paramValue = XmlUtils.getNodeText(childElement);
          strings.put(paramName, paramValue);
        }
      }
    }
    l.setStrings(strings);
    l.setChilds(childs);
  }

  public ResourceSettings load(InputStream is) throws Exception {
    Document localeDom = XmlUtils.parse(is);

    Resources settings = new Resources();
    Element rootElement = localeDom.getDocumentElement();
    NodeList settingsNodeList = rootElement.getElementsByTagName("settings");
    for (int i = 0; i < settingsNodeList.getLength(); i++)
      loadResourceList((Element) settingsNodeList.item(i), settings);

    Resources delimiter = settings.getChilds().get("delimiter");
    String decimalDelimiter = delimiter.getStrings().get("decimal");
    String listDelimiter = delimiter.getStrings().get("list");

    Resources root = new Resources();
    NodeList resourcesNodeList = rootElement.getElementsByTagName("resources");
    for (int i = 0; i < resourcesNodeList.getLength(); i++)
      loadResourceList((Element) resourcesNodeList.item(i), root);

    return new ResourceSettings(decimalDelimiter, listDelimiter, root);
  }
}
