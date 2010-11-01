package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryEditController extends DeliveryController{


  private static final Logger logger = Logger.getLogger(DeliveryEditController.class);

  private Integer id;

  private Delivery delivery;

  private String smsNotificationAddress;

//  private List<Delivery.Day> days = new ArrayList<Delivery.Day>(7);

  public DeliveryEditController() {
    super();

    String p = getRequestParameter(DELIVERY_PARAM);
    id = p==null || (p = p.trim()).length() == 0 ? null : Integer.parseInt(p);

    try{
      reload();
    }catch (AdminException e){
      addError(e);
    }
  }

  private void reload() throws AdminException{
    User u = getConfig().getUser(getUserName());
    if(id == null || (delivery = getConfig().getDelivery(u.getLogin(), u.getPassword(), id)) == null) {
      logger.error("Delivery is not found with id="+id);
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "delivery.not.found",id);
      return;
    }
    smsNotificationAddress = delivery.getSmsNotificationAddress() == null ? null :
        delivery.getSmsNotificationAddress().getSimpleAddress();
//    days.addAll(Arrays.asList(delivery.getActiveWeekDays()));
  }

  public List<SelectItem> getUniqueDeliveryModes() {
    List<SelectItem> sIs = new LinkedList<SelectItem>();
    for(DeliveryMode m : DeliveryMode.values()) {
      sIs.add(new SelectItem(m, m.toString()));
    }
    return sIs;
  }

//  public List<Delivery.Day> getDays() {
//    return days;
//  }
//
//  public void setDays(List<Delivery.Day> days) {
//    this.days = days;
//  }

  public List<SelectItem> getAllDays() {
    List<SelectItem> result = new ArrayList<SelectItem>(7);
    int i=1;
    for(Delivery.Day d : Delivery.Day.values()) {
      result.add(new SelectItem(d, getLocalizedString("weekday."+i%7)));
      i++;
    }
    return result;
  }

  public String save() {
    if(delivery.getRetryPolicy() != null) {
      if(delivery.getRetryPolicy().equals("")) {
        delivery.setRetryPolicy(null);
      }
      if(!getRetryPoliciesPattern().matcher(delivery.getRetryPolicy()).matches()) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "deliver.illegal_retry_policy", delivery.getRetryPolicy());
        return null;
      }
    }
    if(smsNotificationAddress != null) {
      if(smsNotificationAddress.equals("")) {
        smsNotificationAddress = null;
      }
      if(!Address.validate(smsNotificationAddress)) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "deliver.illegal_sms_address", smsNotificationAddress);
        return null;
      }
      delivery.setSmsNotificationAddress(new Address(smsNotificationAddress));
    }
    if(delivery.getEmailNotificationAddress() != null) {
      if(delivery.getEmailNotificationAddress().equals("")) {
        delivery.setEmailNotificationAddress(null);
      }
    }
    if(!delivery.isSecret()) {
      delivery.setSecretMessage(null);
    }
    if(delivery.getType() == Delivery.Type.SingleText && (delivery.getSingleText() == null || delivery.getSingleText().length() == 0)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "deliver.illegal_single_text");
      return null;
    }
    try{
      User u = getConfig().getUser(getUserName());
      getConfig().modifyDelivery(u.getLogin(), u.getPassword(), delivery);
    }catch (AdminException e){
      addError(e);
      return null;
    }
    return "DELIVERIES";
  }



  public Delivery getDelivery() {
    return delivery;
  }

  public Integer getId() {
    return id;
  }

  public void setId(Integer id) {
    this.id = id;
  }

  public String getSmsNotificationAddress() {
    return smsNotificationAddress;
  }

  public void setSmsNotificationAddress(String smsNotificationAddress) {
    this.smsNotificationAddress = smsNotificationAddress;
  }
}
