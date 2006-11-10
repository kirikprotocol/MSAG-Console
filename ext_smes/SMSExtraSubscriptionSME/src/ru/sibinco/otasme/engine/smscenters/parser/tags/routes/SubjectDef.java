package ru.sibinco.otasme.engine.smscenters.parser.tags.routes;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import java.util.HashSet;
import java.util.Set;

import ru.sibinco.otasme.engine.smscenters.parser.Parser;
import ru.sibinco.otasme.engine.smscenters.parser.RoutesParser;
import ru.sibinco.otasme.engine.smscenters.parser.tags.UnknownTag;
import ru.sibinco.otasme.engine.smscenters.parser.tags.Tag;

/**
 * User: artem
 * Date: Aug 1, 2006
 */

public class SubjectDef extends Tag {

  public static final String QNAME = "subject_def";

  private final String id;
  private final Set masks;

  public SubjectDef(Tag parentTag, Parser parser, Attributes atts) {
    super(parentTag, parser);

    this.id = atts.getValue("id");
    this.masks = new HashSet();
  }

  public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
    ((RoutesParser)getParser()).addMasks(id, masks);
  }

  public void addMask(final String mask) {
    masks.add(mask);
  }

  public void characters(char ch[], int start, int length) throws SAXException {
  }

  public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
    if (qName.equals(Mask.QNAME))
      getParser().setCurrentTag(new Mask(this, getParser(), atts));
    else
      getParser().setCurrentTag(new UnknownTag(this, getParser()));
  }
}
