package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.users.User;
import org.apache.log4j.Logger;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
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
  }

  public List<SelectItem> getUniqueDeliveryModes() {
    List<SelectItem> sIs = new LinkedList<SelectItem>();
    for(DeliveryMode m : DeliveryMode.values()) {
      sIs.add(new SelectItem(m, m.toString()));
    }
    return sIs;
  }


  public String save() {
//    if(dynamicModel.getRowCount() == 0) {
//      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.masks.empty");
//      return null;
//    }
//    Configuration config = getConfig();
//    String user = getUserName();
//
//    List<Address> newMasks = new LinkedList<Address>();
//    for(DynamicTableRow row : dynamicModel.getRows()) {
//      String mask = (String) row.getValue("mask");
//      if(!Address.validate(mask)) {
//        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "validation.msisdn");
//        return null;
//      }
//      newMasks.add(new Address(mask));
//    }
//
//    try{
//      delivery.clearMasks();
//      for(Address a : newMasks) {
//        delivery.addMask(a);
//      }
//    }catch (AdminException e) {
//      addError(e);
//      return  null;
//    }
//
//    try{
//      if(delivery.getDeliveryId() != null) {
//        config.updateDelivery(delivery, user);
//      }else {
//        config.addDelivery(delivery, user);
//      }
//
//      return "REGION";
//    }catch (AdminException e){
//      addError(e);
      return null;
//    }
//
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
