package ru.sibinco.smsx.services.calendar.timezones.parsers.tags.timezones;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.smsx.utils.xml.Parser;
import ru.sibinco.smsx.utils.xml.Tag;
import ru.sibinco.smsx.utils.xml.UnknownTag;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class Config extends Tag {

  public static final String QNAME = "config";

  public Config(Tag parentTag, Parser parser) {
    super(parentTag, parser);
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(Param.QNAME))
      getParser().setCurrentTag(new Param(this, "", atts, getParser()));
    else if (qName.equals(Section.QNAME))
      getParser().setCurrentTag(new Section(this, getParser(), atts));
    else
      getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
