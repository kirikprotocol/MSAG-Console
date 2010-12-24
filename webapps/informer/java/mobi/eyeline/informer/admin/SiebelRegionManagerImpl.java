package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
import mobi.eyeline.informer.util.Address;

/**
* @author Artem Snopkov
*/
class SiebelRegionManagerImpl implements SiebelRegionManager {

  private AdminContext context;

  SiebelRegionManagerImpl(AdminContext context) {
    this.context = context;
  }

  public Region getRegion(Address msisdn) throws AdminException {
    return context.getRegion(msisdn);
  }
}
