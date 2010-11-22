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
public class MessagesByDeliveriesController extends LongOperationController {
  private User currentUser;
  private boolean initError = false;
  private MessagesByDeliveriesTotals totals;
  private List<MessagesByDeliveriesRecord> records;
  private DeliveryStatFilter filter;
  boolean fullMode = false;

  private String nameFilter;

  public MessagesByDeliveriesController() {
    super();
    totals = new MessagesByDeliveriesTotals();
    records = Collections.synchronizedList(new ArrayList<MessagesByDeliveriesRecord>());
    filter = new DeliveryStatFilter();
    initUser();    
  }

  private void initUser() {
    currentUser = getConfig().getUser(getUserName());
    if (currentUser == null) initError = true;
    else if (!currentUser.hasRole(User.INFORMER_ADMIN_ROLE)) {
      filter.setUser(currentUser.getLogin());
    } else {
      filter.setUser(null);
    }
  }

  public DeliveryStatFilter getFilter() {
    return filter;
  }

  public void clearFilter() {
    reset();
    totals.reset();
    records.clear();
    initUser();
    filter.setFromDate(null);
    filter.setTillDate(null);
    nameFilter = null;
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
    for (User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }


  @Override
  public void execute(final Configuration config, final Locale locale) throws InterruptedException, AdminException {
    totals.reset();
    records.clear();

    DeliveryFilter deliveryFilter = new DeliveryFilter();
    deliveryFilter.setStartDateFrom(filter.getFromDate());
    deliveryFilter.setEndDateTo(filter.getTillDate());
    if (filter.getUser() != null) {
      deliveryFilter.setUserIdFilter(filter.getUser());
    }
    if (nameFilter != null && nameFilter.trim().length() > 0) {
      deliveryFilter.setNameFilter(nameFilter);
    }


    setCurrentAndTotal(0, config.countDeliveries(getCurrentUser().getLogin(), getCurrentUser().getPassword(), deliveryFilter));

    deliveryFilter.setResultFields(DeliveryFields.Name, DeliveryFields.UserId);

    config.getDeliveries(getCurrentUser().getLogin(), getCurrentUser().getPassword(), deliveryFilter, 1000,
        new Visitor<DeliveryInfo>() {
          public boolean visit(DeliveryInfo deliveryInfo) throws AdminException {
            final int deliveryId = deliveryInfo.getDeliveryId();
            DeliveryStatistics stat = config.getDeliveryStats(getCurrentUser().getLogin(), getCurrentUser().getPassword(), deliveryId);
            DeliveryStatusHistory hist = config.getDeliveryStatusHistory(getCurrentUser().getLogin(), getCurrentUser().getPassword(), deliveryId);
            Date startDate = null;
            Date endDate = null;
            for (DeliveryStatusHistory.Item item : hist.getHistoryItems()) {
              if (item.getStatus() == DeliveryStatus.Active) {
                startDate = item.getDate();
                endDate = null;
              } else if (item.getStatus() == DeliveryStatus.Finished) {
                endDate = item.getDate();
              }
            }

            User owner = config.getUser(deliveryInfo.getUserId());
            MessagesByDeliveriesRecord r = new MessagesByDeliveriesRecord(owner, deliveryInfo, stat, startDate, endDate);
            records.add(r);
            getTotals().add(r);
            setCurrent(getCurrent() + 1);
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
          Collections.sort(records, records.get(0).getRecordsComparator(sortOrder));
        }

        List<MessagesByDeliveriesRecord> result = new LinkedList<MessagesByDeliveriesRecord>();
        for (Iterator<MessagesByDeliveriesRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          MessagesByDeliveriesRecord r = i.next();
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

  public boolean isFullMode() {
    return fullMode;
  }

  public void setFullMode(boolean fullMode) {
    this.fullMode = fullMode;
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {

    for (int i = 0, recordsSize = records.size(); i < recordsSize; i++) {
      MessagesByDeliveriesRecord r = records.get(i);
      if (i == 0) r.printCSVheader(writer, fullMode);
      r.printWithChildrenToCSV(writer, fullMode);
    }
  }

  public MessagesByDeliveriesTotals getTotals() {
    return totals;
  }
}
