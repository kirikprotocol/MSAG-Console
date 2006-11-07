package ru.sibinco.otasme.engine.template.parser;

import org.apache.log4j.Category;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import ru.sibinco.otasme.engine.template.Templates;
import ru.sibinco.otasme.engine.template.parser.tags.Tag;
import ru.sibinco.otasme.engine.template.parser.tags.TemplatesTag;

/**
 * User: artem
 * Date: 25.10.2006
 */

final class TemplatesParser extends DefaultHandler {
  private final static Category log = Category.getInstance(TemplatesParser.class);

  private Tag currentTag = null;
  private final Templates templates;

  public TemplatesParser(Templates templates) {
    this.templates = templates;
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
    if (currentTag != null)
      currentTag = currentTag.endElement();
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equalsIgnoreCase(TemplatesTag.QNAME)) {
      currentTag = new TemplatesTag(null, templates);
      return;
    }

    if (currentTag != null)
      currentTag = currentTag.startElement(qName, atts);
    else {
      System.out.println("Can't read XML file with templates. Wrong format.");
      log.error("Can't read XML file with templates. Wrong format.");
    }
  }
}
