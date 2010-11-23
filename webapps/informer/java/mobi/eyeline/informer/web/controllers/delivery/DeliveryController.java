package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryController extends InformerController {

  public static final String DELIVERY_PARAM = "delivery";

  public static final String DELIVERY_IDS_PARAM = "delivery_ids";

  public static final String DELIVERY_COMEBACK_PARAM = "delivery_comeback";

  protected final Configuration config;

  protected final User user;

  public DeliveryController() {
    this.config = getConfig();
    this.user = config.getUser(getUserName());
  }

  public String getDeliveryParam() {
    return DELIVERY_PARAM;
  }

  public String getDeliveryComeBackParam() {
    return DELIVERY_COMEBACK_PARAM;
  }

  public List<SelectItem> getUniqueStatuses() {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale());
    List<SelectItem> result = new ArrayList<SelectItem>();
    result.add(new SelectItem(null));
    for (DeliveryStatus s : DeliveryStatus.values()) {
      result.add(new SelectItem(s.toString(), bundle.getString("delivery.status." + s)));
    }
    return result;
  }

  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> ss = new LinkedList<SelectItem>();
    ss.add(new SelectItem("",""));
    for(User u : config.getUsers()) {
      ss.add(new SelectItem(u.getLogin(), u.getLogin()));
    }
    return ss;
  }

  public List<SelectItem> getAllDays() {
    List<SelectItem> result = new ArrayList<SelectItem>(7);
    int i = 1;
    for (Delivery.Day d : Delivery.Day.values()) {
      result.add(new SelectItem(d, getLocalizedString("weekday." + i % 7)));
      i++;
    }
    return result;
  }

  public boolean isAllowUssdPushDeliveries() {
    return config.isAllowUssdPushDeliveries();
  }

  public List<SelectItem> getUniqueDeliveryModes() {
    List<SelectItem> sIs = new LinkedList<SelectItem>();
    for (DeliveryMode m : DeliveryMode.values()) {
      sIs.add(new SelectItem(m, m.toString()));
    }
    return sIs;
  }

}
