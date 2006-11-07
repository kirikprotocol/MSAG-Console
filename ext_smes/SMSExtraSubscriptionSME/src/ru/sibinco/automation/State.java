package ru.sibinco.automation;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

/**
 * User: artem
 * Date: 27.10.2006
 */
final class State {

  private static int currentId = 0;

  private final HashMap outLinks = new HashMap();
  private final HashMap inLinks = new HashMap();

  private final int id;

  public State() {
    this.id = currentId++;
  }

  int getId() {
    return id;
  }

  public void addInLink(char ch, final State state) {
    getInLinksWithCreation(ch).add(state);
  }

  // Добавляет новую внешнюю ссылку
  public void addOutLink(final char ch, final State state) {
    getOutLinksWithCreation(ch).add(state);
  }

  public Set getOutLinks(final char ch) {
    return (Set)outLinks.get(new Character(ch));
  }

  private Set getOutLinksWithCreation(final char ch) {
    Set states = getOutLinks(ch);
    if (states == null) {
      states = new HashSet();
      outLinks.put(new Character(ch), states);
    }
    return states;
  }

  public Set getOutgoingChars() {
    return outLinks.keySet();
  }

  public HashMap getOutLinks() {
    return outLinks;
  }

  public Set getIncomingChars() {
    return inLinks.keySet();
  }

  public Set getInLinks(final char ch) {
    return (Set)inLinks.get(new Character(ch));
  }

  private Set getInLinksWithCreation(final char ch) {
    Set states = getInLinks(ch);
    if (states == null) {
      states = new HashSet();
      inLinks.put(new Character(ch), states);
    }
    return states;
  }

  public void removeInLink(char ch, State toState) {
    final Set links = getInLinks(ch);
    if (links != null)
      links.remove(toState);
  }

  public void removeOutLink(char ch, State toState) {
    final Set links = getOutLinks(ch);
    if (links != null)
      links.remove(toState);
  }

  public boolean compareInLinksWithState(State state) {
    if (!getIncomingChars().equals(state.getIncomingChars()))
      return false;

    for (Iterator iter = getIncomingChars().iterator(); iter.hasNext(); ) {
      final char ch = ((Character)iter.next()).charValue();
      if (!getInLinks(ch).equals(state.getInLinks(ch)))
        return false;
    }

    return true;
  }

  public void clearOutLinks() {
    outLinks.clear();
  }

  public void clearInLinks() {
    inLinks.clear();
  }

  public boolean equals(Object obj) {
    return this == obj;
  }
}
