package ru.sibinco.otasme.engine.smscenters.parser.tags.routes;

import org.xml.sax.SAXException;
import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.smscenters.parser.tags.Tag;
import ru.sibinco.otasme.engine.smscenters.parser.tags.UnknownTag;
import ru.sibinco.otasme.engine.smscenters.parser.Parser;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class Routes extends Tag {

  public static final String QNAME = "routes";

  public Routes(Tag parentTag, Parser parser) {
    super(parentTag, parser);
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(SubjectDef.QNAME))
      getParser().setCurrentTag(new SubjectDef(this, getParser(), atts));
    else
      getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
