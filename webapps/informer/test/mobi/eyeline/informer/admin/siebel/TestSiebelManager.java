package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
/**
 * @author Aleksandr Khalitov
 */
class TestSiebelManager extends SiebelManager{

  TestSiebelManager(TestSiebelDataProvider pv, SiebelDeliveries deliveries, SiebelRegionManager regionManager) throws AdminException {
    super(pv, deliveries, regionManager);
  }

  TestSiebelManager(SiebelDeliveries deliveries, SiebelRegionManager regionManager) throws AdminException {
    super(new TestSiebelDataProvider(), deliveries, regionManager);
  }


}
