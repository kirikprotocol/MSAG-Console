package ru.sibinco.otasme.engine.smscenters.parser.tags.routes;

import org.xml.sax.SAXException;
import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.smscenters.parser.tags.UnknownTag;
import ru.sibinco.otasme.engine.smscenters.parser.tags.Tag;
import ru.sibinco.otasme.engine.smscenters.parser.Parser;

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
