package ru.sibinco.otasme.engine.template.parser.tags;

import org.xml.sax.Attributes;
import ru.sibinco.otasme.engine.template.Automation;
import ru.sibinco.otasme.engine.template.State;

import java.util.ArrayList;

/**
 * User: artem
 * Date: 25.10.2006
 */

public class StatesTag extends Tag{

  public static final String QNAME = "states";

  public StatesTag(Tag parentTag, Automation a, ArrayList states, Attributes atts) {
    super(parentTag);
    for (int i=0; i< Integer.parseInt(atts.getValue("count")); i++)
      states.add(new State(a));
  }

  public Tag startElement(String qname, Attributes atts) {
    return null;
  }
}
