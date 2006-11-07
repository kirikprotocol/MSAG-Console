package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.template.State;

import java.util.ArrayList;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class BaseStateTag extends Tag {
  public static final String QNAME="basestate";

  private final State currentState;
  private final ArrayList states;

  public BaseStateTag(Tag parentTag, ArrayList states, Attributes atts) {
    super(parentTag);
    this.states = states;
    currentState = (State)states.get(Integer.parseInt(atts.getValue("id")));
  }

  public Tag startElement(String qname, Attributes atts) {
    if (qname.equalsIgnoreCase(CharTag.QNAME))
      return new CharTag(this, states, currentState, atts);

    return null;
  }
}
