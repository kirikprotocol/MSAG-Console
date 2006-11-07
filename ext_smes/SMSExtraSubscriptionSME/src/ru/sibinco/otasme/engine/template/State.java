package ru.sibinco.otasme.engine.template;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * User: artem
 * Date: 25.10.2006
 */
public final class State {
  private final HashMap outLinks = new HashMap();
  private Automation automation;

  public State(Automation automation) {
    this.automation = automation;
  }

  public Automation getAutomation() {
    return automation;
  }

  public void addOutLinks(final char ch, final ArrayList links) {
    final State[] outStates = new State[links.size()];
    for (int i = 0; i < links.size(); i++)
      outStates[i] = (State)links.get(i);
    outLinks.put(new Character(ch), outStates);
  }

  public State[] getOutLinks(final char ch) {
    return (State[])outLinks.get(new Character(ch));
  }

  public boolean isFinal() {
    return outLinks.isEmpty();
  }
}
