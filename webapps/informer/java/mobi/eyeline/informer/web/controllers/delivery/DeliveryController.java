package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.util.*;

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
    result.add(new SelectItem("",""));
    for (DeliveryStatus s : DeliveryStatus.values()) {
      result.add(new SelectItem(s.toString(), bundle.getString("delivery.status." + s)));
    }
    Collections.sort(result, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return result;
  }

  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> ss = new LinkedList<SelectItem>();
    ss.add(new SelectItem("",""));
    for(User u : config.getUsers()) {
      ss.add(new SelectItem(u.getLogin(), u.getLogin()));
    }
    Collections.sort(ss, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return ss;
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

  public boolean isAllowUssdPushDeliveries() {
    return config.isAllowUssdPushDeliveries();
  }

  public List<SelectItem> getUniqueDeliveryModes() {
    List<SelectItem> sIs = new LinkedList<SelectItem>();
    for (DeliveryMode m : DeliveryMode.values()) {
      sIs.add(new SelectItem(m, m.toString()));
    }
    Collections.sort(sIs, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return sIs;
  }

}
