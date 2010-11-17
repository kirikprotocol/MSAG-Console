package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
/**
 * @author Aleksandr Khalitov
 */
public class TestSiebelManager extends SiebelManager{

  public TestSiebelManager(SiebelDeliveries deliveries, SiebelRegionManager regionManager) throws AdminException {
    super(deliveries, regionManager);
  }

}
