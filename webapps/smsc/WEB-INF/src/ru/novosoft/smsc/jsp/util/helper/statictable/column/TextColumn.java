package ru.novosoft.smsc.jsp.util.helper.statictable.column;

import ru.novosoft.smsc.jsp.util.helper.statictable.Column;

/**
 * User: artem
 * Date: 20.12.2006
 */
public class TextColumn extends Column {
  private String title;

  public TextColumn(String uId, String title) {
    super(uId);
    this.title = title;
  }

  public TextColumn(String uId, String title, boolean sortable) {
    super(uId, sortable);
    this.title = title;
  }

  public TextColumn(String uId, String title, boolean sortable, int width) {
    super(uId, sortable, width);
    this.title = title;
  }

  public void setTitle(String title) {
    this.title = title;
  }

  public String getTitle() {
    return title;
  }
}
