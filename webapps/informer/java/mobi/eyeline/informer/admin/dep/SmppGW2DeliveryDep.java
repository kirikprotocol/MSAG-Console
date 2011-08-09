package mobi.eyeline.informer.admin.dep;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smppgw.SmppGWConfigManager;
import mobi.eyeline.informer.admin.smppgw.SmppGWProvider;
import mobi.eyeline.informer.admin.smppgw.SmppGWRoute;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGW2DeliveryDep {

  private final SmppGWConfigManager smppGwManager;

  public SmppGW2DeliveryDep(SmppGWConfigManager smppGwManager) {
    this.smppGwManager = smppGwManager;
  }

  private boolean canBeDroped(int deliveryId) throws AdminException {
    for(SmppGWProvider p : smppGwManager.getProviderSettings().getProviders()) {
      for(SmppGWRoute r : p.getRoutes()) {
        if(r.getDeliveryId() == deliveryId) {
          return false;
        }
      }
    }
    return true;
  }

  public void removeDelivery(int deliveryId) throws AdminException {
    if(!canBeDroped(deliveryId)) {
      throw new IntegrityException("delivery_remove_smpp_gw_intersection");
    }
  }

  public void archivateDelivery(int deliveryId) throws AdminException {
    if(!canBeDroped(deliveryId)) {
      throw new IntegrityException("delivery_archivate_smpp_gw_intersection");
    }
  }

}
