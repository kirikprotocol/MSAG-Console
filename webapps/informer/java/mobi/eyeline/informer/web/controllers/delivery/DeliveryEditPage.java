package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.io.File;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryEditPage extends InformerController implements CreateDeliveryPage{

  private Delivery delivery;

  private File tmpFile;

  private boolean smsNotificationCheck;

  private boolean emailNotificationCheck;

  public DeliveryEditPage(Delivery delivery, File tmpFile) {
    this.delivery = delivery;
    this.tmpFile = tmpFile;
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException {
    if(smsNotificationCheck) {
      if(delivery.getSmsNotificationAddress() == null) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.sms.incorrect");
        return null;
      }
    }
    if(emailNotificationCheck) {
      if(delivery.getEmailNotificationAddress() == null) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.email.incorrect");
        return null;
      }
    }
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
    User u = config.getUser(user);
    DeliveryFilter filter = new DeliveryFilter();
    filter.setNameFilter(new String[]{delivery.getName()});
    final boolean[] intersection = new boolean[]{false};
    config.getDeliveries(u.getLogin(), u.getPassword(), filter, 1, new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        intersection[0] = true;
        return false;
      }
    });
    if(intersection[0]) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "delivery.name.exist");
      return null;
    }


    if(!smsNotificationCheck) {
      delivery.setSmsNotificationAddress(null);
    }
    if(!emailNotificationCheck) {
      delivery.setEmailNotificationAddress(null);      
    }

    return new ProcessDeliveryPage(delivery, tmpFile, config, locale, user);
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

  public String getPageId() {
    return "DELIVERY_CREATE_EDIT";
  }

  public boolean isEmailNotificationCheck() {
    return emailNotificationCheck;
  }

  public void setEmailNotificationCheck(boolean emailNotificationCheck) {
    this.emailNotificationCheck = emailNotificationCheck;
  }

  public boolean isSmsNotificationCheck() {
    return smsNotificationCheck;
  }

  public void setSmsNotificationCheck(boolean smsNotificationCheck) {
    this.smsNotificationCheck = smsNotificationCheck;
  }

  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  public void cancel() {
    tmpFile.delete();
  }

  public Delivery getDelivery() {
    return delivery;
  }
}
