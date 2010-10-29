package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
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
public class DeliveriesStatsController extends LongOperationController {
  private User currentUser;
  private boolean initError=false;
  private List<DeliveriesStatRecord> records;
  private DeliveryStatFilter filter;

  private String nameFilter;

  public DeliveriesStatsController() {
    super();
    records = new ArrayList<DeliveriesStatRecord>();
    filter=new DeliveryStatFilter();
    initUser();
  }

  private void initUser() {
    currentUser = getConfig().getUser(getUserName());
    if(currentUser ==null) initError = true;
    else if(!currentUser.hasRole(User.INFORMER_ADMIN_ROLE)) {
      filter.setUser(currentUser.getLogin());
    }
    else {
      filter.setUser(null);
    }
  }

  public DeliveryStatFilter getFilter() {
    return filter;
  }
  public void clearFilter() {
    reset();
    records.clear();
    initUser();
    filter.setFromDate(null);
    filter.setTillDate(null);
  }


  public String getNameFilter() {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter) {
    this.nameFilter = nameFilter;
  }

  public User getCurrentUser() {
    return currentUser;
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }



  public boolean isAdmin() {
    return currentUser.hasRole(User.INFORMER_ADMIN_ROLE);
  }



  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(""));
    for(User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }


  @Override
  public void execute(final Configuration config, final Locale locale) throws InterruptedException, AdminException {
    records.clear();

    DeliveryFilter deliveryFilter = new DeliveryFilter();
    deliveryFilter.setStartDateFrom(filter.getFromDate());
    deliveryFilter.setEndDateTo(filter.getTillDate());
    if(filter.getUser()!=null) {
      deliveryFilter.setUserIdFilter(new String[]{filter.getUser()});
    }
    if(nameFilter!=null && nameFilter.trim().length()>0) {
      deliveryFilter.setNameFilter(new String[]{nameFilter});
    }


    setCurrentAndTotal(0,config.countDeliveries(getCurrentUser().getLogin(),getCurrentUser().getPassword(),deliveryFilter));

    deliveryFilter.setResultFields(new DeliveryFields[]{DeliveryFields.Name,DeliveryFields.UserId});

    config.getDeliveries(getCurrentUser().getLogin(),getCurrentUser().getPassword(),deliveryFilter,1000,
        new Visitor<DeliveryInfo>() {
        public boolean visit(DeliveryInfo deliveryInfo ) throws AdminException {
            final int deliveryId = deliveryInfo.getDeliveryId();

            DeliveryStatistics stat = config.getDeliveryStats(getCurrentUser().getLogin(),getCurrentUser().getPassword(),deliveryId);
            records.add(new DeliveriesStatRecord(deliveryInfo,stat));

            setCurrent(getCurrent()+1);
            return !isCancelled();
          }
        }
    );
  }

  public DataTableModel getRecords() {

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        // Сортируем записи
        if (sortOrder != null && !records.isEmpty()) {
          Collections.sort(records, new Comparator<DeliveriesStatRecord>() {

            public int compare(DeliveriesStatRecord o1, DeliveriesStatRecord o2) {

              final int mul = sortOrder.isAsc() ? 1 : -1;
              if (sortOrder.getColumnId().equals("name")) {
                return mul*o1.getInfo().getName().compareTo(o2.getInfo().getName());
              }
              else if (sortOrder.getColumnId().equals("userId")) {
                return mul*o1.getInfo().getUserId().compareTo(o2.getInfo().getUserId());
              }
              else if (sortOrder.getColumnId().equals("status")) {
                return mul*o1.getStat().getDeliveryState().getStatus().compareTo(o2.getStat().getDeliveryState().getStatus());
              }
              else if (sortOrder.getColumnId().equals("new")) {
                return o1.getStat().getNewMessages() >= o2.getStat().getNewMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("process")) {
                return o1.getStat().getProcessMessages() >= o2.getStat().getProcessMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("delivered")) {
                return o1.getStat().getDeliveredMessages() >= o2.getStat().getDeliveredMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("falied")) {
                return o1.getStat().getFailedMessages() >= o2.getStat().getFailedMessages() ? mul : -mul;
              }
              else if (sortOrder.getColumnId().equals("expired")) {
                return o1.getStat().getExpiredMessages() >= o2.getStat().getExpiredMessages() ? mul : -mul;
              }
              
              //todo dates
              return 0;
            }
          });
        }

        List<DeliveriesStatRecord> result = new LinkedList<DeliveriesStatRecord>();
        for (Iterator<DeliveriesStatRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          DeliveriesStatRecord r = i.next();
          if (--startPos < 0) {
            result.add(r);
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

    for(DeliveriesStatRecord r : records) {
      r.printCSV(writer);
    }
  }


}
