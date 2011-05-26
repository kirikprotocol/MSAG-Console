package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.page_calendar.PageCalendarModel;

import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryWeeklyController extends DeliveryController {

  private final User logined;

  private String user;

  private String status;

  public DeliveryWeeklyController() {
    super();
    this.logined = config.getUser(getUserName());
  }

  public void clearFilter() {
    user = null;
    status = null;
  }

  public void query() {}

  private void getDeliveries(Date start, Date end, final Collection<Delivery> result) throws AdminException {
    DeliveryFilter filter = new DeliveryFilter();
    if (!isUserInAdminRole()) {
      filter.setUserIdFilter(logined.getLogin());
    }else if(user != null && user.length() > 0) {
      filter.setUserIdFilter(user);      
    }
    if(status != null && status.length()>0) {
      filter.setStatusFilter(DeliveryStatus.valueOf(status));
    }
    filter.setStartDateFrom(start);
    filter.setStartDateTo(end);
    config.getDeliveries(logined.getLogin(), filter, 1000, new Visitor<Delivery>() {
      public boolean visit(Delivery value) throws AdminException {
        result.add(value);
        return true;
      }
    });
  }

  public String getStatus() {
    return status;
  }

  public void setStatus(String status) {
    this.status = status;
  }

  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    this.user = user;
  }


  public PageCalendarModel getCalendarModel() {

    return new PageCalendarModel() {

      private final List<Delivery> ds = new LinkedList<Delivery>();

      private final SimpleDateFormat df = new SimpleDateFormat("ddMMyyyy");

      private final long oneDay = 24 * 60 * 60 * 1000;

      public void updateVisiblePeriod(Date startDate, Date endDate) {
        try {
          Calendar c = Calendar.getInstance();
          c.setTime(startDate);
          c.set(Calendar.HOUR_OF_DAY, 0);
          c.set(Calendar.MINUTE, 0);
          c.set(Calendar.SECOND, 0);
          c.set(Calendar.MILLISECOND, 0);
          startDate = c.getTime();
          c.setTime(endDate);
          c.set(Calendar.HOUR_OF_DAY, 0);
          c.set(Calendar.MINUTE, 0);
          c.set(Calendar.SECOND, 0);
          c.set(Calendar.MILLISECOND, 0);
          c.add(Calendar.DAY_OF_MONTH, 1);
          c.add(Calendar.MILLISECOND, -1);
          endDate = c.getTime();
          getDeliveries(startDate, endDate, ds);
        } catch (AdminException e) {
          addError(e);
        }
      }

      public Object getValue(Date date) {
        String d = df.format(date);
        List<Delivery> toReturn = new LinkedList<Delivery>();
        for (Delivery di : ds) {
          if (df.format(di.getStartDate()).equals(d)) {
            toReturn.add(di);
          }
        }
        return toReturn;
      }
    };
  }

}
