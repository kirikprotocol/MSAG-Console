package ru.sibinco.calendarsme.engine.timezones.parsers.tags.timezones;

import ru.sibinco.calendarsme.engine.timezones.parsers.tags.Tag;
import ru.sibinco.calendarsme.engine.timezones.parsers.tags.UnknownTag;
import ru.sibinco.calendarsme.engine.timezones.parsers.Parser;
import org.xml.sax.SAXException;
import org.xml.sax.Attributes;

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
    if (qName.equals(Param.QNAME) && name != null && name.equalsIgnoreCase("subjects"))
      getParser().setCurrentTag(new Param(this, atts, getParser()));
    else
      getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
