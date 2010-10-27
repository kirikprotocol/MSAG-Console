package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
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
public class MessagesByRecStatsController extends LongOperationController {
  private User user;
  private boolean initError=false;
  private List<MessagesByRecStatRecord> records;


  private Date fromDate;
  private Date tillDate;
  private String msisdn;
  private String allowedUser;

  public MessagesByRecStatsController() {
    super();
    records = new ArrayList<MessagesByRecStatRecord>();

    initUser();
  }

  private void initUser() {
    user = getConfig().getUser(getUserName());
    if(user==null) initError = true;
    else if(!user.hasRole(User.INFORMER_ADMIN_ROLE)) {
      allowedUser = user.getLogin();
    }
    else {
      allowedUser = null;
    }
  }

  public void clearFilter() {
    initUser();
    setFromDate(null);
    setTillDate(null);
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
    for(User u : getConfig().getUsers()) {
      ret.add(new SelectItem(u.getLogin()));
    }
    return ret;
  }


  @Override
  public void execute(final Locale locale) throws InterruptedException, AdminException {
    records.clear();

    DeliveryFilter deliveryFilter = new DeliveryFilter();
    deliveryFilter.setStartDateFrom(fromDate);
    deliveryFilter.setEndDateTo(tillDate);
    if(allowedUser!=null) {
      deliveryFilter.setUserIdFilter(new String[]{allowedUser});
    }

    setCurrentAndTotal(0,getConfig().countDeliveries(getUser().getLogin(),getUser().getPassword(),deliveryFilter));

    deliveryFilter.setResultFields(new DeliveryFields[]{DeliveryFields.Name,DeliveryFields.UserId,DeliveryFields.StartDate,DeliveryFields.EndDate});

    getConfig().getDeliveries(getUser().getLogin(),getUser().getPassword(),deliveryFilter,1000,
        new Visitor<DeliveryInfo>() {
          public boolean visit(DeliveryInfo deliveryInfo ) throws AdminException {
            final int deliveryId = deliveryInfo.getDeliveryId();
            final String name = deliveryInfo.getName();
            final String userId = deliveryInfo.getUserId();

            MessageFilter messageFilter = new MessageFilter();

            messageFilter.setDeliveryId(deliveryId);
            messageFilter.setStartDate(fromDate==null ? deliveryInfo.getStartDate() : fromDate);
            messageFilter.setEndDate(tillDate==null ? deliveryInfo.getEndDate() : tillDate);
            messageFilter.setMsisdnFilter(new String[]{msisdn});
            messageFilter.setFields(new MessageFields[]{MessageFields.Date,MessageFields.State,MessageFields.Text, MessageFields.ErrorCode});

            getConfig().getMessagesStates(getUser().getLogin(),getUser().getPassword(),messageFilter,1000,
                new Visitor<MessageInfo>() {
                  public boolean visit(MessageInfo messageInfo) throws AdminException {
                    String errString=null;
                    Integer errCode = messageInfo.getErrorCode();
                    if(errCode!=null) {
                      try {
                        errString = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("informer.errcode."+errCode);
                      }
                      catch (MissingResourceException ex) {
                        errString = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale).getString("informer.errcode.unknown");
                      }
                    }
                    MessagesByRecStatRecord rec = new MessagesByRecStatRecord(deliveryId,userId,name,messageInfo.getText(),messageInfo.getDate(),messageInfo.getState(),errString);
                    records.add(rec);
                    return true;
                  }
                }
            );
            setCurrent(getCurrent()+1);
            return true;
          }
        }
    );
  }



  public DataTableModel getRecords() {

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        // Сортируем записи
        if (sortOrder != null && !records.isEmpty()) {
          Collections.sort(records, new Comparator<MessagesByRecStatRecord>() {

            public int compare(MessagesByRecStatRecord o1, MessagesByRecStatRecord o2) {

              final int mul = sortOrder.isAsc() ? 1 : -1;
              if (sortOrder.getColumnId().equals("name")) {
                return mul*o1.getName().compareTo(o2.getName());
              }
              if (sortOrder.getColumnId().equals("userId")) {
                return mul*o1.getUserId().compareTo(o2.getUserId());
              }
              if (sortOrder.getColumnId().equals("text")) {
                if(o1.getText()==null && o1.getText()==null) return 0;
                if(o1.getText()==null) return mul;
                return mul*o1.getText().compareTo(o2.getText());
              }
              if (sortOrder.getColumnId().equals("deliveryDate")) {
                return mul*o1.getDeliveryDate().compareTo(o2.getDeliveryDate());
              }
              if (sortOrder.getColumnId().equals("state")) {
                return mul*o1.getState().compareTo(o2.getState());
              }
              if (sortOrder.getColumnId().equals("errorString")) {
                return mul*o1.getErrorString().compareTo(o2.getErrorString());
              }
              return 0;
            }
          });
        }

        List<MessagesByRecStatRecord> result = new LinkedList<MessagesByRecStatRecord>();
        for (Iterator<MessagesByRecStatRecord> i = records.iterator(); i.hasNext() && count > 0;) {
          MessagesByRecStatRecord r = i.next();
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

    for(MessagesByRecStatRecord r : records) {
      r.printCSV(writer);
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

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  //

}