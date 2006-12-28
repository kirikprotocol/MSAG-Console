package ru.novosoft.smsc.jsp.util.tables.table;

/**
 * User: artem
 * Date: 20.12.2006
 */

public abstract class InputCell extends Cell {

  private final String name = UniqueStringGenerator.generateUniqueString();

  protected InputCell(SimpleTableBean bean, Object object) {
    super(object);
    if (bean != null)
      bean.registerCell(this);
  }

  public String getName() {
    return name;
  }

  public abstract void setValue(String value);
}
