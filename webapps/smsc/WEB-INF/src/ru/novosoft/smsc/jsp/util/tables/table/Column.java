package ru.novosoft.smsc.jsp.util.tables.table;

/**
 * User: artem
 * Date: 18.12.2006
 */
public abstract class Column {

  private final int width;
  private final String id;

  private boolean sortable;

  public Column(String uId, SimpleTableBean simpleTableBean) {
    this(uId, simpleTableBean, false);
  }

  public Column(String uId, SimpleTableBean simpleTableBean, boolean sortable) {
    this(uId, simpleTableBean, sortable, 0);
  }

  public Column(String uId, SimpleTableBean simpleTableBean, boolean sortable, int width) {
    simpleTableBean.addColumn(this);
    this.id = uId;
    this.sortable = sortable;
    this.width = width;
  }

  public void setSortable(boolean sortable) {
    this.sortable = sortable;
  }

  public boolean isSortable() {
    return sortable;
  }

  public int getWidth() {
    return width;
  }

  public String getId() {
    return id;
  }
}
