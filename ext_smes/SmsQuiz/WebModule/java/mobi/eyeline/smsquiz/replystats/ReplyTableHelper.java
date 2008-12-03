package mobi.eyeline.smsquiz.replystats;

import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.OrderType;
import ru.novosoft.smsc.jsp.util.helper.statictable.Row;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataSource;

import java.text.SimpleDateFormat;
import java.util.*;

/**
 * author: alkhal
 * Date: 12.11.2008
 */

public class ReplyTableHelper extends PagedStaticTableHelper {


  private final TextColumn msisdnColumn = new TextColumn(ReplyDataSource.MSISDN, "smsquiz.label.abonent", true, 25);
  private final TextColumn dateColumn = new TextColumn(ReplyDataSource.REPLY_DATE, "smsquiz.label.replyDate", true, 25);
  private final TextColumn messageColumn = new TextColumn(ReplyDataSource.MESSAGE, "smsquiz.label.message", true, 50);

  public static final String DEFAULT_SORT = ReplyDataSource.MSISDN;

  private static final SimpleDateFormat DATE_FORMAT = new SimpleDateFormat("dd.MM.yy HH:mm");

  private DataSource ds;

  private int totalSize = 0;

  private String sortOrder;

  private String prevSortOrder;

  private QueryResultSet replies;

  private Filter filter;

  public ReplyTableHelper(String uid) {
    super(uid, false);

    addColumn(msisdnColumn);
    addColumn(dateColumn);
    addColumn(messageColumn);
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
      if (sortOrder != null && (prevSortOrder == null || !prevSortOrder.equals(sortOrder))) {
        replies = null;
        prevSortOrder = sortOrder;
      }

      if(replies == null) {
        replies = ds.query(new ReplyQuery(getMaxRows(), filter, sortOrder, 0));
      }

      clear();
      for (int i = start; i < start + size && i < replies.size(); i++) {
        final DataItem item = replies.get(i);

        final Row row = createNewRow();

        final String msisdn = (String) item.getValue(ReplyDataSource.MSISDN);
        row.addCell(msisdnColumn, new StringCell(msisdn, msisdn, false));
        row.addCell(dateColumn, new StringCell(msisdn,
            convertDateToString((Date) item.getValue(ReplyDataSource.REPLY_DATE)), false));
        row.addCell(messageColumn, new StringCell(msisdn,
            (String) item.getValue(ReplyDataSource.MESSAGE), false));
      }
      totalSize = replies.getTotalSize();

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

  public void reset() {
    setStartPosition(0);
    replies = null;
  }

}

