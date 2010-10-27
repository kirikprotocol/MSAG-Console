package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.controllers.InformerController;
import mobi.eyeline.informer.web.controllers.LongOperationController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public abstract class DeliveryStatController extends LongOperationController {
  private User user;
  private boolean initError=false;
  private DeliveryStatFilter filter;
  private AggregationType aggregation;

  private TreeMap<Date, AggregatedStatRecord> recordsMap;
  private List<AggregatedStatRecord> records;



  public DeliveryStatController() {
    super();
    aggregation= AggregationType.HOUR;
    filter = new DeliveryStatFilter();
    initUser();
    records = new ArrayList<AggregatedStatRecord>();
    recordsMap = new TreeMap<Date, AggregatedStatRecord>();
  }

  private void initUser() {
    user = getConfig().getUser(getUserName());
    if(user==null) initError = true;
    else if(!user.hasRole(User.INFORMER_ADMIN_ROLE)) {
      filter.setUser(getUserName());
    }
    else {
      filter.setUser(null);
    }
  }

  public void clearFilter() {
    initUser();
    filter.setFromDate(null);
    filter.setTillDate(null);
    filter.setTaskId(null);
  }


  public void setUser(User user) {
    this.user = user;
  }

  public User getUser() {
    return user;
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public DeliveryStatFilter getFilter() {
    return filter;
  }

  public void setFilter(DeliveryStatFilter filter) {
    this.filter = filter;
  }

  public AggregationType getAggregation() {
    return aggregation;
  }

  public void setAggregation(AggregationType aggregation) {
    this.aggregation = aggregation;
  }

  public List<SelectItem> getAggregations() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for(AggregationType a : AggregationType.values()) {
      ret.add(new SelectItem(a));
    }
    return ret;
  }

  public boolean isAdmin() {
    return user.hasRole(User.INFORMER_ADMIN_ROLE);
  }

  protected void clearRecords() {
      recordsMap.clear();
      records.clear();
  }

  protected void loadFinished() {
      records.addAll(recordsMap.values());
  }

  protected AggregatedStatRecord getRecord(Date time) {
     return recordsMap.get(time);
  }
  protected void putRecord(AggregatedStatRecord newRecord) {
     recordsMap.put(newRecord.getStartCalendar().getTime(),newRecord);
  }


  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(""));
    for(User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }

  abstract void loadRecords(final Locale locale) throws AdminException, InterruptedException;

  @Override
  public void execute(final Locale locale) throws Exception {
    clearRecords();
    try {
      loadRecords(locale);
    }
    catch (Exception e) {
      clearRecords();
      throw e;
    }
    finally {
      loadFinished();
    }
  }


  public DataTableModel getRecords() {
    //loadRecords();
    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        // Сортируем записи
        if (sortOrder != null && !records.isEmpty()) {
          Collections.sort(records, records.get(0).getRecordsComparator(sortOrder));
        }

        List<AggregatedStatRecord> result = new LinkedList<AggregatedStatRecord>();
        for (Iterator<AggregatedStatRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          AggregatedStatRecord r = i.next();
          if (--startPos < 0) {
            result.add(r);
            List<AggregatedStatRecord> innerRecords = r.getInnerRows();
            if(innerRecords!=null && !innerRecords.isEmpty() && sortOrder!=null) {
              Collections.sort(r.getInnerRows(), innerRecords.get(0).getRecordsComparator(sortOrder));
            }
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return records.size();
      }
    };
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {
    //loadRecords();
    for(AggregatedStatRecord r : records) {
      r.printWithChildrenToCSV(writer);
    }
  }


}