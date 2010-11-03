package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatFilter;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.config.Configuration;
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
  private TimeAggregationType aggregation;

  private Map<Object, AggregatedRecord> recordsMap;
  private List<AggregatedRecord> records;


  boolean fullMode =false;

  public DeliveryStatController() {
    super();
    aggregation= TimeAggregationType.HOUR;
    filter = new DeliveryStatFilter();
    initUser();
    records = Collections.synchronizedList(new ArrayList<AggregatedRecord>());
    recordsMap = Collections.synchronizedMap(new TreeMap<Object, AggregatedRecord>());
  }

  protected void initUser() {
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
    reset();
    clearRecords();
    initUser();
    filter.setFromDate(null);
    filter.setTillDate(null);
    filter.setTaskId(null);
    setFullMode(false);
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

  public TimeAggregationType getAggregation() {
    return aggregation;
  }

  public void setAggregation(TimeAggregationType aggregation) {
    this.aggregation = aggregation;
  }

  public List<SelectItem> getAggregations() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for(TimeAggregationType a : TimeAggregationType.values()) {
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

  protected AggregatedRecord getRecord(Object key) {
     return recordsMap.get(key);
  }
  protected void putRecord(AggregatedRecord newRecord) {
     recordsMap.put(newRecord.getAggregationKey(),newRecord);
  }


  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(""));
    for(User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }

  abstract void loadRecords(Configuration config, final Locale locale) throws AdminException, InterruptedException;

  @Override
  public void execute(Configuration config, final Locale locale) throws Exception {
    clearRecords();
    try {
      loadRecords(config,locale);
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

        List<AggregatedRecord> result = new LinkedList<AggregatedRecord>();
        for (Iterator<AggregatedRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          AggregatedRecord r = i.next();
          if (--startPos < 0) {
            result.add(r);
            List<AggregatedRecord> innerRecords = r.getInnerRows();
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

  public boolean isFullMode() {
    return fullMode;
  }

  public void setFullMode(boolean fullMode) {
    this.fullMode = fullMode;
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {    
    for (int i = 0, recordsSize = records.size(); i < recordsSize; i++) {
      AggregatedRecord r = records.get(i);
      if(i==0) {
        r.printCSVheader(writer, fullMode);
      }
      r.printWithChildrenToCSV(writer, fullMode);
    }
  }




  @Override
  public void reset() {
    super.reset();    //To change body of overridden methods use File | Settings | File Templates.
    clearRecords();
  }

}