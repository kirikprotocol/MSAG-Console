package ru.sibinco.smsx.utils.operators;

import java.util.Collection;
import java.util.LinkedList;

/**
 * User: artem
 * Date: 09.07.2007
 */

public class Operator {
  private final String name;
  private Collection<String> masks = new LinkedList<String>();

  public Operator(String name) {
    this.name = name;
  }

  public void addMask(String mask) {
    this.masks.add(mask);
  }

  public String getName() {
    return name;
  }

  public Collection<String> getMasks() {
    return masks;
  }
}
