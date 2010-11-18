package mobi.eyeline.informer.admin.siebel.impl;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.util.Address;

/**
 * @author Aleksandr Khalitov
 */
public interface SiebelRegionManager {

  public Region getRegion(Address msisdn) throws AdminException;

}
