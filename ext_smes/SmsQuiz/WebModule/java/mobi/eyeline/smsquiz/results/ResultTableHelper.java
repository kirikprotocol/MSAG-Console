package mobi.eyeline.smsquiz.results;

import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.OrderType;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.Row;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.DataSource;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * author: alkhal
 * Date: 12.11.2008
 */
public class ResultTableHelper extends PagedStaticTableHelper {


  private final TextColumn msisdnColumn = new TextColumn(ResultDataSource.MSISDN, "smsquiz.label.abonent", true, 20);
  private final TextColumn deliveryDateColumn = new TextColumn(ResultDataSource.REPLY_DATE, "smsquiz.label.deliveryDate", true, 20);
  private final TextColumn replyDateColumn = new TextColumn(ResultDataSource.DELIVERY_DATE, "smsquiz.label.replyDate", true, 20);
  private final TextColumn messageColumn = new TextColumn(ResultDataSource.MESSAGE, "smsquiz.label.message", true, 20);
  private final TextColumn categoryColumn = new TextColumn(ResultDataSource.CATEGORY, "smsquiz.label.category", true, 20);

  public static final String DEFAULT_SORT = "+" + ResultDataSource.MSISDN;

  private static final SimpleDateFormat DATE_FORMAT = new SimpleDateFormat("dd.MM.yy HH:mm");

  private DataSource ds;

  private int totalSize = 0;

  private String sortOrder = "";

  private Filter filter;


  public ResultTableHelper(String uid) {
    super(uid, false);

    addColumn(msisdnColumn);
    addColumn(deliveryDateColumn);
    addColumn(replyDateColumn);
    addColumn(messageColumn);
    addColumn(categoryColumn);
  }

  private void buildSortOrder() {
    SortOrderElement[] sortOrderElements = getSortOrder();
    if ((sortOrderElements != null) && (sortOrderElements.length > 0)) {
      SortOrderElement element = sortOrderElements[0];
      if (element != null) {
        sortOrder = element.getColumnId();
        if (sortOrder != null) {
          if (element.getOrderType() != OrderType.ASC) {
            sortOrder = "-" + sortOrder;
          }
          return;
        }
      }
    }
    sortOrder = DEFAULT_SORT;
  }

  protected void fillTable(int start, int size) throws TableHelperException {
    try {
      buildSortOrder();
      final QueryResultSet messages = ds.query(new ResultQuery(size, filter, sortOrder, start));

      for (int i = 0; i < messages.size(); i++) {
        final DataItem item = messages.get(i);

        final Row row = createNewRow();

        final String msisdn = (String) item.getValue(ResultDataSource.MSISDN);
        row.addCell(msisdnColumn, new StringCell(msisdn, msisdn, false));
        row.addCell(replyDateColumn, new StringCell(msisdn,
            convertDateToString((Date) item.getValue(ResultDataSource.REPLY_DATE)), false));
        row.addCell(deliveryDateColumn, new StringCell(msisdn,
            convertDateToString((Date) item.getValue(ResultDataSource.DELIVERY_DATE)), false));
        row.addCell(messageColumn, new StringCell(msisdn,
            (String) item.getValue(ResultDataSource.MESSAGE), false));
        row.addCell(categoryColumn, new StringCell(msisdn,
            (String) item.getValue(ResultDataSource.CATEGORY), false));
      }
      totalSize = messages.getTotalSize();

    } catch (Exception e) {
      throw new TableHelperException(e);
    }
  }

  protected int calculateTotalSize() throws TableHelperException {
    return totalSize;
  }

  public DataSource getDs() {
    return ds;
  }

  public void setDs(DataSource ds) {
    this.ds = ds;
  }

  public int getTotalSize() {
    return totalSize;
  }

  public void setTotalSize(int totalSize) {
    this.totalSize = totalSize;
  }

  private String convertDateToString(Date date) {
    return DATE_FORMAT.format(date);
  }

  public Filter getFilter() {
    return filter;
  }

  public void setFilter(Filter filter) {
    this.filter = filter;
  }
}

