package ru.novosoft.smsc.jsp.util.tables.table;

/**
 * User: artem
 * Date: 18.12.2006
 */
public abstract class Column {

  private boolean selectable;
  private final int width;
  private final String id;

  public Column(SimpleTableBean simpleTableBean) {
    this(simpleTableBean, false);
  }

  public Column(SimpleTableBean simpleTableBean, boolean selectable) {
    this(simpleTableBean, selectable, 0);
  }

  public Column(SimpleTableBean SimpleTableBean, boolean selectable, int width) {
    this.id = SimpleTableBean.addColumn(this);
    this.selectable = selectable;
    this.width = width;
  }

  public void setSelectable(boolean selectable) {
    this.selectable = selectable;
  }

  public boolean isSelectable() {
    return selectable;
  }

  public int getWidth() {
    return width;
  }

  String getId() {
    return id;
  }
}
