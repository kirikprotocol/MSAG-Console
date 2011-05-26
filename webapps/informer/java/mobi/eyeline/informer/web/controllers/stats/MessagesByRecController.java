package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.LongOperationController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 22.10.2010
 * Time: 14:05:42
 */
public class MessagesByRecController extends LongOperationController {
  private User user;
  private boolean initError = false;
  private List<MessagesByRecRecord> records;


  private Date fromDate;
  private Date tillDate;
  private Address msisdn;
  private String allowedUser;

  public MessagesByRecController() {
    super();
    records = Collections.synchronizedList(new ArrayList<MessagesByRecRecord>());
    initUser();
  }

  private void initUser() {
    user = getConfig().getUser(getUserName());
    if (user == null) initError = true;
    else if (!user.hasRole(User.INFORMER_ADMIN_ROLE)) {
      allowedUser = user.getLogin();
    } else {
      allowedUser = null;
    }
  }

  public void clearFilter() {
    reset();
    try{
      lock.lock();
      records.clear();
    }finally {
      lock.unlock();
    }
    initUser();
    setFromDate(null);
    setTillDate(null);
    msisdn = null;

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


  public boolean isAdmin() {
    return user.hasRole(User.INFORMER_ADMIN_ROLE);
  }


  public List<SelectItem> getUsers() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(""));
    for (User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }

  private Lock lock = new ReentrantLock();

  @Override
  public void execute(final Configuration config, final Locale locale) throws InterruptedException, AdminException {

    DeliveryFilter deliveryFilter = new DeliveryFilter();
    deliveryFilter.setStartDateFrom(fromDate);
    deliveryFilter.setEndDateTo(tillDate);
    if (allowedUser != null) {
      deliveryFilter.setUserIdFilter(allowedUser);
    }

    setCurrentAndTotal(0, config.countDeliveries(getUser().getLogin(), deliveryFilter));
    try{
      lock.lock();
      records.clear();
      config.getDeliveries(getUser().getLogin(), deliveryFilter, 1000,
          new Visitor<Delivery>() {
            public boolean visit(Delivery Delivery) throws AdminException {
              final int deliveryId = Delivery.getId();
              final String name = Delivery.getName();
              final String userId = Delivery.getOwner();
              final User owner = config.getUser(userId);

              MessageFilter messageFilter = new MessageFilter(deliveryId, fromDate == null ? Delivery.getStartDate() : fromDate,
                  tillDate != null ? tillDate : Delivery.getEndDate() != null ? Delivery.getEndDate() : new Date());
              messageFilter.setMsisdnFilter(msisdn.getSimpleAddress());

              config.getMessagesStates(getUser().getLogin(), messageFilter, 1000,
                  new Visitor<Message>() {
                    public boolean visit(Message message) throws AdminException {
                      String errString = null;
                      Integer errCode = message.getErrorCode();
                      if (errCode != null) {
                        try {
                          errString = ResourceBundle.getBundle("mobi.eyeline.informer.admin.SmppStatus", locale).getString("informer.errcode." + errCode);
                        }
                        catch (MissingResourceException ex) {
                          errString = ResourceBundle.getBundle("mobi.eyeline.informer.admin.SmppStatus", locale).getString("informer.errcode.unknown");
                        }
                      }
                      MessagesByRecRecord rec = new MessagesByRecRecord(message.getAbonent().getSimpleAddress(), deliveryId, userId, owner, name, message.getText(), message.getDate(), message.getState(), errString);
                      records.add(rec);
                      return !isCancelled();
                    }
                  }
              );
              setCurrent(getCurrent() + 1);
              return !isCancelled();
            }
          }
      );
    }finally {
      lock.unlock();
    }
  }


  public synchronized DataTableModel getRecords() {

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        try{
          lock.lock();
          // Сортируем записи
          if (sortOrder != null && !records.isEmpty()) {
            Collections.sort(records, new Comparator<MessagesByRecRecord>() {

              public int compare(MessagesByRecRecord o1, MessagesByRecRecord o2) {

                final int mul = sortOrder.isAsc() ? 1 : -1;
                if (sortOrder.getColumnId().equals("name")) {
                  return mul * o1.getName().compareTo(o2.getName());
                }
                if (sortOrder.getColumnId().equals("userId")) {
                  return mul * o1.getUserId().compareTo(o2.getUserId());
                }
                if (sortOrder.getColumnId().equals("text")) {
                  if (o1.getText() == null && o1.getText() == null) return 0;
                  if (o1.getText() == null) return mul;
                  return mul * o1.getText().compareTo(o2.getText());
                }
                if (sortOrder.getColumnId().equals("deliveryDate")) {
                  return mul * o1.getDeliveryDate().compareTo(o2.getDeliveryDate());
                }
                if (sortOrder.getColumnId().equals("state")) {
                  return mul * o1.getState().compareTo(o2.getState());
                }
                if (sortOrder.getColumnId().equals("errorString")) {
                  return mul * o1.getErrorString().compareTo(o2.getErrorString());
                }
                return 0;
              }
            });
          }

          List<MessagesByRecRecord> result = new LinkedList<MessagesByRecRecord>();
          for (Iterator<MessagesByRecRecord> i = records.iterator(); i.hasNext() && count > 0;) {
            MessagesByRecRecord r = i.next();
            if (--startPos < 0) {
              result.add(r);
              count--;
            }
          }
          return result;
        }finally {
          lock.unlock();
        }
      }

      public int getRowsCount() {
        try{
          lock.lock();
          return records.size();
        }finally {
          lock.unlock();
        }
      }
    };
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {
    try{
      lock.lock();
      for (MessagesByRecRecord r : records) {
        r.printCSV(writer);
      }
    }finally {
      lock.unlock();
    }
  }


  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  public Date getFromDate() {
    return fromDate;
  }

  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public Address getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(Address msisdn) {
    this.msisdn = msisdn;
  }

  //

}