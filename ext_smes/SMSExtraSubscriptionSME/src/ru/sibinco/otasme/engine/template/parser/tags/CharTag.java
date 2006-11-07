package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.template.State;

import java.util.ArrayList;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class CharTag extends Tag {

  public static final String QNAME = "char";

  private final State currentState;
  private final ArrayList states;
  private final char ch;
  private final ArrayList links = new ArrayList();

  public CharTag(Tag parentTag, ArrayList states, State currentState, Attributes atts) {
    super(parentTag);
    this.currentState = currentState;
    this.states = states;
    ch = atts.getValue("value").charAt(0);
  }

  public Tag startElement(String qname, Attributes atts) {
    if (qname.equalsIgnoreCase(StateTag.QNAME))
      return new StateTag(this, states, links, atts);

    return null;
  }

  public Tag endElement() {
    currentState.addOutLinks(ch, links);
    links.clear();
    return super.endElement();
  }
}
