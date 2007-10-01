package ru.sibinco.smsx.engine.smpphandler.redirects;

import com.eyeline.sme.utils.xml.reader.sax.Parser;
import com.eyeline.sme.utils.xml.reader.sax.Tag;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import java.util.Collection;

/**
 * User: artem
 * Date: 24.01.2007
 */
class RedirectsTag extends Tag{

  public static final String QNAME = "redirects";
  public static final String REDIRECT_QNAME = "redirect";

  private final Collection redirects;

  RedirectsTag(Tag parentTag, Parser parser, Collection redirects) {
    super(parentTag, parser);
    this.redirects = redirects;
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(REDIRECT_QNAME))
      redirects.add(new ru.sibinco.smsx.engine.smpphandler.redirects.Redirect(atts.getValue("format"), atts.getValue("prefix"), atts.getValue("new_prefix"), atts.getValue("address")));

    setUnknownTag();
  }
}
