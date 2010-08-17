package ru.novosoft.smsc.web.components.paged_table;

/**
 * @author Artem Snopkov
 */
public class Column {

  private final String name;
  private final String title;
  private int width = 100;
  private String align = "left";
  private String actionListener;
  private boolean sortable;

  public Column(String name, String title) {
    this.name = name;
    this.title = title;
  }

  public String getName() {
    return name;
  }

  public String getTitle() {
    return title;
  }

  public int getWidth() {
    return width;
  }

  public void setWidth(int width) {
    this.width = width;
  }

  public String getActionListener() {
    return actionListener;
  }

  public void setActionListener(String actionListener) {
    this.actionListener = actionListener;
  }

  public String getAlign() {
    return align;
  }

  public void setAlign(String align) {
    this.align = align;
  }

  public boolean isSortable() {
    return sortable;
  }

  public void setSortable(boolean sortable) {
    this.sortable = sortable;
  }

  public boolean equals(Object o) {
    if (o instanceof Column) {
      Column c = (Column)o;
      return c.name.equals(name);
    }
    return false;
  }
}
