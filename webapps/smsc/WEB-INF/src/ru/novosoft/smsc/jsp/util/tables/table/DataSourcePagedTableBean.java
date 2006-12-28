package ru.novosoft.smsc.jsp.util.tables.table;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 21.12.2006
 */
public abstract class DataSourcePagedTableBean extends PagedTableBean {

  private DataSource dataSource = null;


  public DataSource getDataSource() {
    return dataSource;
  }

  public void setDataSource(DataSource dataSource) {
    this.dataSource = dataSource;
  }

  protected abstract Query createQuery(int start, int size, Column orderByColumn);

  protected abstract void fillRow(Row row, DataItem dataItem);

  protected final void fillTable(int start, int size) throws AdminException {
    if (dataSource == null)
      throw new AdminException("Data source is null");

    final QueryResultSet rs = dataSource.query(createQuery(start, size, getSortedColumn()));

    for (int i=0; i<rs.size(); i++)
      fillRow(createNewRow(), rs.get(i));

    setTotalSize(rs.getTotalSize());
  }

  protected int calculateTotalSize() {
    return getTotalSizeInt();
  }

}
