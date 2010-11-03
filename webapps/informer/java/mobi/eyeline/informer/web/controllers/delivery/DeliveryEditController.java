package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.users.User;
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

  private String comeBackParam;

  public DeliveryEditController() {
    super();

    String p = getRequestParameter(DELIVERY_PARAM);
    id = p==null || (p = p.trim()).length() == 0 ? null : Integer.parseInt(p);

    p = getRequestParameter(DELIVERY_COMEBACK_PARAM);
    if(p != null && p.length() != 0) {
      comeBackParam = p;
    }

    try{
      reload();
    }catch (AdminException e){
      addError(e);
    }
  }

  public String getComeBackParam() {
    return comeBackParam;
  }

  public void setComeBackParam(String comeBackParam) {
    this.comeBackParam = comeBackParam;
  }

  private void reload() throws AdminException{
    User u = config.getUser(getUserName());
    if(id == null || (delivery = config.getDelivery(u.getLogin(), u.getPassword(), id)) == null) {
      logger.error("Delivery is not found with id="+id);
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "delivery.not.found",id);
    }
  }

  public List<SelectItem> getUniqueDeliveryModes() {
    List<SelectItem> sIs = new LinkedList<SelectItem>();
    for(DeliveryMode m : DeliveryMode.values()) {
      sIs.add(new SelectItem(m, m.toString()));
    }
    return sIs;
  }

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
    if(delivery.getValidityDate() != null) {
      delivery.setValidityPeriod(null);
    }else {
      if(delivery.getValidityPeriod() == null || delivery.getValidityPeriod().length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.validity.empty");
        return null;
      }
    }
    if(!delivery.isRetryOnFail()) {
      delivery.setRetryPolicy(null);
    }else if(delivery.getRetryPolicy() == null || !getRetryPoliciesPattern().matcher(delivery.getRetryPolicy()).matches()) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "deliver.illegal_retry_policy", delivery.getRetryPolicy() == null ? "" : delivery.getRetryPolicy());
      return null;
    }
    if(delivery.getEmailNotificationAddress() != null) {
      if(delivery.getEmailNotificationAddress().equals("")) {
        delivery.setEmailNotificationAddress(null);
      }
    }
    if(delivery.getType() == Delivery.Type.SingleText && (delivery.getSingleText() == null || delivery.getSingleText().length() == 0)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "deliver.illegal_single_text");
      return null;
    }
    try{
      User u = config.getUser(getUserName());
      config.modifyDelivery(u.getLogin(), u.getPassword(), delivery);
    }catch (AdminException e){
      addError(e);
      return null;
    }
    return comeBackParam != null && comeBackParam.length() != 0 ? comeBackParam : "DELIVERIES";
  }

  public String cancel() {
    String p = comeBackParam != null && comeBackParam.length() > 0 ? comeBackParam : getRequestParameter(DELIVERY_COMEBACK_PARAM);
    return p != null && p.length() >0 ? p : "DELIVERIES";

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
}
