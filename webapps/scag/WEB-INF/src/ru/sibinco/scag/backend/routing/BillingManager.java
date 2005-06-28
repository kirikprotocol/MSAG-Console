package ru.sibinco.scag.backend.routing;

import org.w3c.dom.*;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.*;
import java.util.*;


/**
 * Created by igork Date: 21.09.2004 Time: 18:15:08
 */
public class BillingManager
{
  private final File configFile;
  private final Map rules = new TreeMap();
  private final Transformer transformer;
  private final CharArrayWriter outBuffer = new CharArrayWriter();

  public BillingManager(final File config) throws IOException, ParserConfigurationException, SAXException, TransformerException
  {
    this.configFile = config;
    transformer = TransformerFactory.newInstance().newTransformer();
    transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
    transformer.setOutputProperty(OutputKeys.INDENT, "yes");
    transformer.setOutputProperty(OutputKeys.METHOD, "xml");
    transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");

    final Document rulesDoc = Utils.parse(new FileReader(config));
    final Element rulesElem = rulesDoc.getDocumentElement();
    final NodeList rulesElems = rulesElem.getElementsByTagName("rule");
    for (int i = 0; i < rulesElems.getLength(); i++) {
      final Element ruleElem = (Element) rulesElems.item(i);
      outBuffer.reset();
      final StreamResult outputTarget = new StreamResult(outBuffer);
      final NodeList childNodes = ruleElem.getChildNodes();
      for (int j = 0; j < childNodes.getLength(); j++) {
        final Node currentChild = childNodes.item(j);
        if (Node.ELEMENT_NODE == currentChild.getNodeType())
          transformer.transform(new DOMSource(currentChild), outputTarget);
      }
      final BillingRule rule = new BillingRule(ruleElem.getAttribute("name"), outBuffer.toString());
      rules.put(rule.getId(), rule);
    }
  }

  public Map getRules()
  {
    return rules;
  }

  public void save() throws IOException, UnsupportedEncodingException
  {
    final File configNew = Functions.createNewFilenameForSave(configFile);

    final String encoding = Functions.getLocaleEncoding();
    final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(configNew), encoding));
    Functions.storeConfigHeader(out, "ruleset", "billing-rules.dtd", encoding);
    for (Iterator i = rules.values().iterator(); i.hasNext();) {
      final BillingRule rule = (BillingRule) i.next();
      out.print("  <rule name=\"" + rule.getId() + "\">\n" + rule.getText() + "\n  </rule>");
    }
    Functions.storeConfigFooter(out, "ruleset");
    out.flush();
    out.close();

    Functions.renameNewSavedFileToOriginal(configNew, configFile);
  }
}
