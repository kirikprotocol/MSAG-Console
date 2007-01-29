package ru.sibinco.smsx.services.calendar.timezones.parsers.tags.routes;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.smsx.utils.xml.Parser;
import ru.sibinco.smsx.utils.xml.Tag;
import ru.sibinco.smsx.utils.xml.UnknownTag;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class Mask extends Tag {

  public static final String QNAME = "mask";

  private final String value;

  public Mask(Tag parentTag, Parser parser, Attributes atts) {
    super(parentTag, parser);
    this.value = atts.getValue("value");
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
    if (value != null)  {
      ((SubjectDef)getParentTag()).addMask(value);
    }
    else
      System.out.println("value null");
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    System.out.println("xx");
    getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
