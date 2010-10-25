package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryStatRecord;
import mobi.eyeline.informer.admin.delivery.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.controllers.InformerController;

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
public class DeliveryStatController extends InformerController implements DeliveryStatVisitor {
  private User user;
  private boolean initError=false;
  private DeliveryStatFilter filter;
  private AggregatedDeliveryStatRecord.AggregationType aggregation;
  private Object users;
  private boolean initdata;
  private List<AggregatedDeliveryStatRecord> records;
  private AggregatedDeliveryStatRecord lastRecord=null;
  private boolean expandDetails=true;

  public DeliveryStatController() {
    super();
    aggregation= AggregatedDeliveryStatRecord.AggregationType.HOUR;
    filter = new DeliveryStatFilter();
    initUser();
    initdata=false;
    records = new ArrayList<AggregatedDeliveryStatRecord>();
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
    filter.setUser(null);
    filter.setFromDate(null);
    filter.setTillDate(null);
    filter.setTaskId(null);
  }

  public void query() {
    initdata = true;
  }

  public boolean isInitdata() {
    return initdata;
  }

  public void setInitdata(boolean initdata) {
    this.initdata = initdata;
  }

  public void setUser(User user) {
    this.user = user;
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

  public AggregatedDeliveryStatRecord.AggregationType getAggregation() {
    return aggregation;
  }

  public void setAggregation(AggregatedDeliveryStatRecord.AggregationType aggregation) {
    this.aggregation = aggregation;
  }

  public List<SelectItem> getAggregations() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for(AggregatedDeliveryStatRecord.AggregationType a : AggregatedDeliveryStatRecord.AggregationType.values()) {
      ret.add(new SelectItem(a));
    }
    return ret;
  }

  public boolean isAdmin() {
    return user.hasRole(User.INFORMER_ADMIN_ROLE);
  }

  public boolean isExpandDetails() {
    return expandDetails;
  }

  public void setExpandDetails(boolean expandDetails) {
    this.expandDetails = expandDetails;
  }

  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(""));
    for(User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }


  private void loadRecords() throws AdminException {
    records.clear();
    getConfig().getDeliveryStatProvider().accept(filter,this);
  }


  public boolean visit(DeliveryStatRecord rec, int total, int current) {
    AggregatedDeliveryStatRecord newRecord = new AggregatedDeliveryStatRecord(rec,aggregation,true);
    if(records.size()==0) {
      records.add(newRecord);
    }
    else {
      AggregatedDeliveryStatRecord lastRecord = records.get(records.size()-1);
      if(!lastRecord.add(newRecord)) {
        records.add(newRecord);
      }
    }
    return true;
  }


  public DataTableModel getRecords() {
    try{
      loadRecords();
    }catch (AdminException e){
      addError(e);
      records = Collections.emptyList();
    }
    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        // Сортируем записи
        if (sortOrder != null) {
          Collections.sort(records, new AggregatedRecordsComparator(sortOrder));
        }

        List<AggregatedDeliveryStatRecord> result = new LinkedList<AggregatedDeliveryStatRecord>();
        for (Iterator<AggregatedDeliveryStatRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          AggregatedDeliveryStatRecord r = i.next();
          if (--startPos < 0) {
            result.add(r);
            if(r.getDetails()!=null && sortOrder!=null) {
              Collections.sort(r.getDetails(), new AggregatedRecordsComparator(sortOrder));
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
    try{
      loadRecords();
    }
    catch (AdminException e){
      addError(e);
      records = Collections.emptyList();
    }    
    for(AggregatedDeliveryStatRecord r : records) {
      writer.println(StringEncoderDecoder.toCSVString(
          new Object[]{r.getPeriodString(),"",r.getDelivered(),r.getFailed()}));
      if(r.isParent() && expandDetails && r.getDetails()!=null) {
        for(AggregatedDeliveryStatRecord d : r.getDetails()) {
            writer.println(StringEncoderDecoder.toCSVString(
            new Object[]{"",d.getPeriodString(),d.getDelivered(),d.getFailed()}));
        }
      }
    }        
  }

  class AggregatedRecordsComparator implements  Comparator<AggregatedDeliveryStatRecord> {
    private DataTableSortOrder sortOrder;

    AggregatedRecordsComparator(DataTableSortOrder sortOrder) {
      this.sortOrder = sortOrder;
    }

    public int compare(AggregatedDeliveryStatRecord o1, AggregatedDeliveryStatRecord o2) {
      final int mul = sortOrder.isAsc() ? 1 : -1;
      if (sortOrder.getColumnId().equals("period")) {
        return mul*o1.getStartCalendar().compareTo(o2.getStartCalendar());
      }
      else if (sortOrder.getColumnId().equals("delivered")) {
        Locale l = getLocale();
        return o1.getDelivered() >= o2.getDelivered() ? mul : -mul;
      }
      else if (sortOrder.getColumnId().equals("failed")) {
        return o1.getFailed() >= o2.getFailed() ? mul : -mul;
      }
      return 0;
    }
  }
}


