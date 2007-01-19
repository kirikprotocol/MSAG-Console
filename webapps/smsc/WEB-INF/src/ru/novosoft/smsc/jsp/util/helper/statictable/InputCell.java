package ru.novosoft.smsc.jsp.util.helper.statictable;



/**
 * User: artem
 * Date: 20.12.2006
 */

public abstract class InputCell extends Cell {

  protected InputCell(String id) {
    super(id);
  }

  public abstract void setValue(String value);
}
