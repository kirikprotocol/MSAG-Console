package ru.novosoft.smsc.jsp.util.tables.table.column.sortable;

import ru.novosoft.smsc.jsp.util.tables.table.SimpleTableBean;
import ru.novosoft.smsc.jsp.util.tables.table.Column;

/**
 * User: artem
 * Date: 20.12.2006
 */
public class TextColumn extends Column {
  private final String title;

  public TextColumn(String uId, SimpleTableBean simpleTableBean, String title) {
    super(uId, simpleTableBean);
    this.title = title;
  }

  public TextColumn(String uId, SimpleTableBean simpleTableBean, String title, boolean sortable) {
    super(uId, simpleTableBean, sortable);
    this.title = title;
  }

  public TextColumn(String uId, SimpleTableBean simpleTableBean, String title, boolean sortable, int width) {
    super(uId, simpleTableBean, sortable, width);
    this.title = title;
  }

  public String getTitle() {
    return title;
  }
}
