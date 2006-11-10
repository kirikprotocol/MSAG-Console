package ru.sibinco.otasme.engine.smscenters.parser.tags.smscenters;

import org.xml.sax.SAXException;
import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.smscenters.parser.Parser;
import ru.sibinco.otasme.engine.smscenters.parser.tags.Tag;
import ru.sibinco.otasme.engine.smscenters.parser.tags.UnknownTag;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class Section extends Tag {
  public static final String QNAME = "section";

  private final String name;

  public Section(Tag parentTag, Parser parser, Attributes atts) {
    super(parentTag, parser);
    this.name = atts.getValue("name");
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(Param.QNAME) && name != null && (name.equalsIgnoreCase("subjects") || name.equalsIgnoreCase("masks")))
      getParser().setCurrentTag(new Param(this, name, atts, getParser()));
    else
      getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
