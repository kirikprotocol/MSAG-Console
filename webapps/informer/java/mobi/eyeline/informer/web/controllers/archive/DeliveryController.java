package mobi.eyeline.informer.web.controllers.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.admin.delivery.DeliveryStatusHistory;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;
import mobi.eyeline.informer.web.controllers.delivery.MessageListController;
import mobi.eyeline.informer.web.controllers.stats.ErrorStatsController;
import mobi.eyeline.informer.web.controllers.users.UserEditController;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryController extends InformerController{

  private String comeBackAction;

  private String reqId;

  private Integer deliveryId;

  private Delivery delivery;

  private DeliveryStatusHistory statusHistory;

  private DeliveryStatistics status;

  private boolean ownerExist;

  private boolean secret;

  private String secretMessage;

  private boolean flashSecret;

  private String emailNotificationAddress;
  private String smsNotificationAddress;

  private String error;

  public DeliveryController() {
    comeBackAction = getRequestParameter("action");
    reqId = getRequestParameter("reqId");
    Configuration config = getConfig();
    try {
      if(!config.isArchiveDaemonDeployed() || config.getArchiveDaemonOnlineHost() == null) {
        error = getLocalizedString("archive.daemon.offline");
        return;
      }
    } catch (AdminException e) {
      error = e.getMessage(getLocale());
      return;
    }
    String user = getUserName();
    String id = getRequestParameter("deliveryId");
    if(id == null || id.length() == 0) {            // come back
      String p = getRequestParameter(UserEditController.COME_BACK_PARAMS);
      if(p == null || p.length() == 0) {
        p = getRequestParameter(ErrorStatsController.COME_BACK_PARAMS);
      }
      if(p == null || p.length() == 0) {
        p = getRequestParameter(MessageListController.COME_BACK_PARAMS);
      }
      if(p != null && p.length()>0) {
        String[] ss = p.split(",");
        reqId = ss[0];
        id = ss[1];
        comeBackAction = ss[2];
      }
    }

    if(id != null && id.length()>0) {
      deliveryId = Integer.parseInt(id);
      try{
        delivery = config.getArchiveDelivery(user, deliveryId);
        if(delivery != null) {
          statusHistory = config.getArchiveHistory(user, deliveryId);
          status = config.getArchiveStatistics(user, deliveryId);
          List<User> userList = config.getUsers();
          for(User u : userList) {
            if(u.getLogin().equals(delivery.getOwner())) {
              ownerExist = true;
              break;
            }
          }
          secret = Boolean.valueOf(delivery.getProperty(UserDataConsts.SECRET));
          flashSecret = Boolean.valueOf(delivery.getProperty(UserDataConsts.SECRET_FLASH));
          secretMessage = delivery.getProperty(UserDataConsts.SECRET_TEXT);
          emailNotificationAddress = delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
          smsNotificationAddress = delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
        }else {
          addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "delivery.not.found", "");
        }
      }catch (AdminException e){
        error = e.getMessage(getLocale());
        return;
      }
    }else {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "delivery.not.found", id);
    }

  }

  public String getError() {
    return error;
  }

  public boolean isOffline() {
    return error != null;
  }

  public List<SelectItem> getAllDays() {
    List<SelectItem> result = new ArrayList<SelectItem>(7);
    int i = 1;
    for (Day d : Day.values()) {
      result.add(new SelectItem(d, getLocalizedString("weekday." + i % 7)));
      i++;
    }
    return result;
  }

  public String getActivePeriodStart() {
    return delivery == null ? null : getTime(delivery.getActivePeriodStart());
  }

  public String getActivePeriodEnd() {
    return delivery == null ? null : getTime(delivery.getActivePeriodEnd());
  }

  private static String getTime(Time time) {
    if(time == null) {
      return null;
    }
    int hour = time.getHour();
    int min = time.getMin();
    StringBuilder sb = new StringBuilder(8);
    if (hour < 10) sb.append('0');
    sb.append(hour).append(':');
    if (min < 10) sb.append('0');
    sb.append(min);
    return sb.toString();

  }

  public String getValidityPeriod() {
    return delivery == null ? null : getTime(delivery.getValidityPeriod());
  }

  public boolean isSecret() {
    return secret;
  }

  public String getSecretMessage() {
    return secretMessage;
  }

  public boolean isFlashSecret() {
    return flashSecret;
  }

  public boolean isOwnerExist() {
    return ownerExist;
  }

  public String getComeBackAction() {
    return comeBackAction;
  }

  public void setComeBackAction(String comeBackAction) {
    this.comeBackAction = comeBackAction;
  }

  public String getReqId() {
    return reqId;
  }

  public void setReqId(String reqId) {
    this.reqId = reqId;
  }

  public Integer getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(Integer deliveryId) {
    this.deliveryId = deliveryId;
  }

  public Delivery getDelivery() {
    return delivery;
  }

  public DeliveryStatusHistory getStatusHistory() {
    return statusHistory;
  }

  public DeliveryStatistics getStatus() {
    return status;
  }

  public String getEmailNotificationAddress() {
    return emailNotificationAddress;
  }

  public String getSmsNotificationAddress() {
    return smsNotificationAddress;
  }

  public boolean isAllowUssdPushDeliveries() {
    return getConfig().isAllowUssdPushDeliveries();
  }

  public String getDeliveryMode() {
    return delivery == null ? null : delivery.getDeliveryMode().toString();
  }

  public void statForSelected(ActionEvent e) {
    if(delivery != null) {
      getRequest().put("delivery", delivery.getId());
    }
  }

  public String getMessagesComeBackName() {
    return MessageListController.COME_BACK;
  }

  public String getMessagesComeBackParamsName() {
    return MessageListController.COME_BACK_PARAMS;
  }
}
