package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;

import java.util.ArrayList;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class StateTag extends Tag {

  public static final String QNAME = "state";

  public StateTag(Tag parentTag, ArrayList states, ArrayList links, Attributes atts) {
    super(parentTag);
    links.add(states.get(Integer.parseInt(atts.getValue("id"))));
  }

  public Tag startElement(String qname, Attributes atts) {
    return null;
  }
}
