package ru.sibinco.smsx.services.redirector.redirects.parsers.tags;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import ru.sibinco.smsx.utils.xml.Parser;
import ru.sibinco.smsx.utils.xml.Tag;
import ru.sibinco.smsx.utils.xml.UnknownTag;

import java.util.List;

/**
 * User: artem
 * Date: 24.01.2007
 */
public class Redirects extends Tag{

  public static final String QNAME = "redirects";

  private final List redirects;

  public Redirects(Tag parentTag, Parser parser, List redirects) {
    super(parentTag, parser);
    this.redirects = redirects;
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(Redirect.QNAME))
      getParser().setCurrentTag(new Redirect(this, getParser(), atts, redirects));
    else
      getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
