package ru.novosoft.smsc.jsp.util.tables.table.column.sortable;

import ru.novosoft.smsc.jsp.util.tables.table.SimpleTableBean;
import ru.novosoft.smsc.jsp.util.tables.table.column.SortableColumn;

/**
 * User: artem
 * Date: 20.12.2006
 */
public class TextColumn extends SortableColumn {
  private final String title;

  public TextColumn(SimpleTableBean simpleTableBean, String title) {
    super(simpleTableBean);
    this.title = title;
  }

  public TextColumn(SimpleTableBean simpleTableBean, String title, boolean sortable) {
    super(simpleTableBean);
    setSortable(sortable);
    this.title = title;
  }

  public TextColumn(SimpleTableBean simpleTableBean, String title, boolean sortable, int width) {
    super(simpleTableBean, sortable, width);
    setSortable(sortable);
    this.title = title;
  }

  public String getTitle() {
    return title;
  }
}
