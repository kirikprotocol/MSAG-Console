package ru.sibinco.smsx.stats.beans;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.util.helper.statictable.*;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.ImageColumn;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.sibinco.smsx.stats.backend.SmsxRequest;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import javax.servlet.http.HttpServletRequest;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public abstract class RequestsTableHelper extends PagedStaticTableHelper{


  private static final Category logger = Category.getInstance(RequestsTableHelper.class);

  private Locale locale;

  private final ImageColumn checkColumn = new ImageColumn("checkColumn", "/images/ico16_checked_sa.gif", "", false, 1);
  private final TextColumn fromDateColumn = new TextColumn("fromDate", "smsx.stat.from", true, 25);
  private final TextColumn tillColumn = new TextColumn("tillDate", "smsx.stat.till", true,25);
  private final TextColumn stateColumn = new TextColumn("state", "smsx.stat.state", true, 24);
  private final TextColumn actionColumn = new TextColumn("action", "smsx.stat.action", false,25);

  private ResourceBundle bundle;

  private final RequestFilter requestFilter;

  RequestsTableHelper(String uid, RequestFilter requestFilter) {
    super(uid, false);
    this.requestFilter = requestFilter;
    setPageSize(20);
    addColumn(checkColumn);
    addColumn(fromDateColumn);
    addColumn(tillColumn);
    addColumn(stateColumn);
    addColumn(actionColumn);
  }

  public void setLocale(Locale locale) {
    this.locale = locale;
  }

  private void initResources() {
    if(locale == null) {
      locale = Locale.ENGLISH;
    }
    try{
      bundle = ResourceBundle.getBundle("locales.messages",locale);
    }catch (MissingResourceException e){
      logger.error(e,e);
    }
  }

  private String getLocString(String key) {
    if(bundle == null) {
      return key;
    }
    try{
      return bundle.getString(key);
    }catch (MissingResourceException e){
      logger.error(e,e);
      return key;
    }
  }



  protected abstract List loadRequests() throws StatisticsException;

  boolean isAllowed(SmsxRequest r) {
    return requestFilter == null || requestFilter.isAllowed(r);
  }

  protected void fillTable(int start, int size) throws TableHelperException {
    initResources();
    clear();
    List requests;
    try {
      requests = loadRequests(); //requestManager.getSmsxRequests();
    } catch (StatisticsException e) {
      logger.error(e,e);
      throw new TableHelperException(e);
    }
    {
      Iterator i = requests.iterator();
      while(i.hasNext()) {
        SmsxRequest r = (SmsxRequest)i.next();
        if(!isAllowed(r)) {
          i.remove();
        }
      }
    }

    setTotalSize(requests.size());


    StaticTableHelper.SortOrderElement[] sOs = getSortOrder();
    final StaticTableHelper.SortOrderElement sortOrder = sOs != null && sOs.length>0 ? sOs[0] : null;

    if (sortOrder != null) {
      Collections.sort(requests, new Comparator() {
        public int compare(Object r1, Object r2) {
          SmsxRequest o1 = (SmsxRequest)r1;
          SmsxRequest o2 = (SmsxRequest)r2;
          int mul = sortOrder.getOrderType() == OrderType.ASC ? 1 : -1;
          if ("fromDate".equals(sortOrder.getColumnId())) {
            return (mul) * o1.getFrom().compareTo(o2.getFrom());
          }
          if ("tillDate".equals(sortOrder.getColumnId())) {
            return (mul) * o1.getTill().compareTo(o2.getTill());
          }
          if ("state".equals(sortOrder.getColumnId())) {
            return (mul) * o1.getStatus().toString().compareTo(o2.getStatus().toString());
          }
          return 0;
        }
      });
    }

    for (Iterator i = requests.iterator(); i.hasNext() && size > 0;) {
      SmsxRequest request = (SmsxRequest)i.next();
      if (--start < 0) {
        addRecord(request);
        size--;
      }
    }

  }

  private void addRecord(SmsxRequest request) {
    String id = Integer.toString(request.getId());
    SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");
    final Row row = createNewRow();
    row.addCell(checkColumn, new CheckBoxCell("chb" + id, false));
    row.addCell(fromDateColumn, new StringCell(id, sdf.format(request.getFrom()), request.getStatus() == SmsxRequest.Status.READY));
    row.addCell(tillColumn, new StringCell(id, sdf.format(request.getTill()), request.getStatus() == SmsxRequest.Status.READY));
    SmsxRequest.Status status = request.getStatus();
    String s = getLocString("smsx.stat.request.state." + status.toString());
    if(status == SmsxRequest.Status.IN_PROCESS) {
      s = new StringBuffer().append(s).append(" (").append(request.getProgress()).append("%)").toString();
    }
    row.addCell(stateColumn, new StringCell(id, s, false));
    StringCell cell;
    if(status == SmsxRequest.Status.ERROR || status == SmsxRequest.Status.CANCELED) {
     cell = new StringCell(id, "", false);
    }else if(status == SmsxRequest.Status.IN_PROCESS) {
     cell = new StringCell("cancel_"+id, getLocString("smsx.stat.request.cancel"), true);
    }else {
     cell = new StringCell("download_"+id, getLocString("smsx.stat.request.download"), true);
    }
    row.addCell(actionColumn, cell);

  }

  public List getSelectedRequests(HttpServletRequest request) {
    final ArrayList result = new ArrayList();
    for (Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext(); ) {
      final String paramName = (String)iter.next();
      if (paramName.startsWith("chb"))
        result.add(Integer.valueOf(paramName.substring(3)));
    }
    return result;
  }



  protected int calculateTotalSize() throws TableHelperException {
    return getTotalSize();
  }
}
