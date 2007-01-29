package ru.sibinco.smsx.utils.xml;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * User: artem
 * Date: 24.01.2007
 */
public class UnknownTag extends Tag{

  public UnknownTag(Tag parentTag, Parser parser) {
    super(parentTag, parser);
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
