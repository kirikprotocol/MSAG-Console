package ru.novosoft.smsc.infosme.backend.tables.messages;

import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.jsp.util.helper.statictable.OrderType;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.Row;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;

import javax.servlet.http.HttpServletRequest;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

/**
 * author: alkhal
 * Date: 11.11.2008
 */
public class MessagesTableHelper extends PagedStaticTableHelper  {

  private final TextColumn checkColumn = new TextColumn("checkColumn", "", false);
  private final TextColumn msisdnColumn = new TextColumn(MessageDataSource.MSISDN, "infosme.label.abonent", true, 20);
  private final TextColumn stateColumn = new TextColumn(MessageDataSource.STATE, "infosme.label.msg_status", true, 20);
  private final TextColumn dateColumn = new TextColumn(MessageDataSource.DATE, "infosme.label.date", true, 20);
  private final TextColumn messageColumn = new TextColumn(MessageDataSource.MESSAGE,"infosme.label.message", true, 30);

  public static final String DEFAULT_SORT = null;

  private static final SimpleDateFormat DATE_FORMAT = new SimpleDateFormat("dd.MM.yy HH:mm");

  private DataSource ds;

  private int totalSize = 0;

  private String sortOrder, prevSortOrder;

  private Filter filter;

  private boolean addCheck = true;

  private QueryResultSet messages;


  public MessagesTableHelper(String uid, boolean addCheck) {
    super(uid, false);
    this.addCheck = addCheck;
    if(addCheck){
      addColumn(checkColumn);
    }
    addColumn(msisdnColumn);
    addColumn(stateColumn);
    addColumn(dateColumn);
    addColumn(messageColumn);
  }

  private void buildSortOrder() {
    SortOrderElement[] sortOrderElements = getSortOrder();
    if((sortOrderElements!=null)&&(sortOrderElements.length>0)) {
      SortOrderElement element = sortOrderElements[0];
      if(element!=null) {
        sortOrder=element.getColumnId();
        if(sortOrder!=null) {
          if(element.getOrderType()!= OrderType.ASC) {
            sortOrder="-"+sortOrder;
          }
          return;
        }
      }
    }
    sortOrder=DEFAULT_SORT;
  }

  protected void fillTable(int start, int size) throws TableHelperException {
    try{

      buildSortOrder();
      if (sortOrder != null && (prevSortOrder == null || !prevSortOrder.equals(sortOrder))) {
        messages = null;
        prevSortOrder = sortOrder;
      }

      if (messages == null) {
        messages = ds.query(new MessageQuery(getMaxRows(), filter, sortOrder, 0));
      }

      clear();
      for (int i = start; i < start + size && i < messages.size(); i++) {
        final DataItem item = messages.get(i);

        final Row row = createNewRow();

        final String msisdn = (String) item.getValue(MessageDataSource.MSISDN);
        if(addCheck) {
          row.addCell(checkColumn, new CheckBoxCell("chb" + item.getValue(MessageDataSource.ID), false));
        }
        row.addCell(msisdnColumn, new StringCell(msisdn, msisdn, false));
        row.addCell(dateColumn, new StringCell(msisdn,
            convertDateToString((Date)item.getValue(MessageDataSource.DATE)), false));
        row.addCell(stateColumn, new StringCell(msisdn,
            getStateName((Message.State)item.getValue(MessageDataSource.STATE)), false));
        row.addCell(messageColumn, new StringCell(msisdn,
            (String)item.getValue(MessageDataSource.MESSAGE), false));
      }
      totalSize = messages.getTotalSize();

    } catch(Exception e) {
      throw new TableHelperException(e);
    }
  }

  protected int calculateTotalSize() throws TableHelperException {
    return totalSize;
  }

  public DataSource getDs() {
    return ds;
  }

  public void setDs(AbstractDataSource ds) {
    this.ds = ds;
  }

  private String getStateName(Message.State state) {
    if (state == Message.State.UNDEFINED)
      return "ALL";
    else if (state == Message.State.NEW)
      return "NEW";
    else if (state == Message.State.WAIT)
      return "WAIT";
    else if (state == Message.State.ENROUTE)
      return "ENROUTE";
    else if (state == Message.State.DELIVERED)
      return "DELIVERED";
    else if (state == Message.State.EXPIRED)
      return "EXPIRED";
    else if (state == Message.State.FAILED)
      return "FAILED";
    else if (state == Message.State.DELETED)
      return "DELETED";

    return "";
  }

  public void reset() {
    messages = null;
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

  public List getSelectedMessagesList(HttpServletRequest request) {
    final ArrayList result = new ArrayList();
    for (Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext(); ) {
      final String paramName = (String)iter.next();
      if (paramName.startsWith("chb"))
        result.add(paramName.substring(3));
    }
    return result;
  }

  public String[] getSelectedMessagesArray(HttpServletRequest request) {
    List list = getSelectedMessagesList(request);
    return (String[])list.toArray(new String[list.size()]);
  }
}
