package ru.novosoft.smsc.web.components.dynamic_table;

/**
 * @author Artem Snopkov
 */
abstract class Column  {

  private String name;
  private String width = "99%";
  private String title;

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getWidth() {
    return width;
  }

  public void setWidth(String width) {
    this.width = width;
  }

  public String getTitle() {
    return title;
  }

  public void setTitle(String title) {
    this.title = title;
  }
}
