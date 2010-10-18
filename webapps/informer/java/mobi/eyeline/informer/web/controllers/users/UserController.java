package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
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
    for(User.Status s : User.Status.values()) {
      ret.add(new SelectItem(s));
    }
    return ret;
  }

  public List<SelectItem> getDaysItems() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for(int i=1;i<=7;i++) {
      ret.add(new SelectItem(new Integer(i%7),getLocalizedString("weekday."+i%7)));
    }
    return ret;
  }

  public List<SelectItem> getDeliveryTypes() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(User.DeliveryType.SMS,getLocalizedString("user.edit.deliveryType."+User.DeliveryType.SMS)));
    ret.add(new SelectItem(User.DeliveryType.USSD_PUSH,getLocalizedString("user.edit.deliveryType."+User.DeliveryType.USSD_PUSH)));
    ret.add(new SelectItem(User.DeliveryType.USSD_PUSH_VIA_VLR,getLocalizedString("user.edit.deliveryType."+User.DeliveryType.USSD_PUSH_VIA_VLR)));
    return ret;
  }

  public List<SelectItem> getPolicies() throws AdminException {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    List<RetryPolicy> policies = getConfig().getRetryPolicies();
    for(RetryPolicy p : policies) {
      ret.add(new SelectItem(p.getPolicyId()));
    }
    return ret;
  }

  public List<SelectItem> getRegionsList() {
    List<SelectItem> regions = new ArrayList<SelectItem>();
    for(Region r : getConfig().getRegions()) {
      regions.add(new SelectItem(r.getRegionId(),r.getName()));
    }
    return regions;
  }

}
