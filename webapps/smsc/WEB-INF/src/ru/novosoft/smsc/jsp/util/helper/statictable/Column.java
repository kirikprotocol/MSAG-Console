package ru.novosoft.smsc.jsp.util.helper.statictable;

/**
 * User: artem
 * Date: 18.12.2006
 */
public abstract class Column {

  private final int width;
  private final String id;

  private boolean sortable;

  public Column(String uId) {
    this(uId, false);
  }

  public Column(String uId, boolean sortable) {
    this(uId, sortable, 0);
  }

  public Column(String uId, boolean sortable, int width) {
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
