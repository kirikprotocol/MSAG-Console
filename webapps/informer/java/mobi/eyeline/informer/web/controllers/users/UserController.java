package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 18.10.2010
 * Time: 15:16:30
 */
public class UserController extends InformerController {

  public UserController() {
    super();
  }

  public List<SelectItem> getStatuses() {
    ArrayList<SelectItem> ret = new ArrayList<SelectItem>();
    for (User.Status s : User.Status.values()) {
      ret.add(new SelectItem(s));
    }
    Collections.sort(ret, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return ret;
  }

  public List<SelectItem> getDaysItems() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for (int i = 1; i <= 7; i++) {
      ret.add(new SelectItem(i % 7, getLocalizedString("weekday." + i % 7)));
    }
    return ret;
  }

  public List<SelectItem> getDeliveryTypes() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(User.DeliveryType.SMS, getLocalizedString("user.edit.deliveryType." + User.DeliveryType.SMS)));
    ret.add(new SelectItem(User.DeliveryType.USSD_PUSH, getLocalizedString("user.edit.deliveryType." + User.DeliveryType.USSD_PUSH)));
    ret.add(new SelectItem(User.DeliveryType.USSD_PUSH_VIA_VLR, getLocalizedString("user.edit.deliveryType." + User.DeliveryType.USSD_PUSH_VIA_VLR)));
    return ret;
  }


  public List<SelectItem> getRegionsList() {
    List<SelectItem> regions = new ArrayList<SelectItem>();
    for (Region r : getConfig().getRegions()) {
      regions.add(new SelectItem(r.getRegionId(), r.getName()));
    }
    Collections.sort(regions, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return regions;
  }

  public boolean isAllowUssdPushDeliveries() {
    return getConfig().isAllowUssdPushDeliveries();
  }

}
