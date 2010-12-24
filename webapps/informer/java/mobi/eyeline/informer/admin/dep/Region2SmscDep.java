package mobi.eyeline.informer.admin.dep;

import mobi.eyeline.informer.admin.*;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.RegionsManager;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.SmscException;
import mobi.eyeline.informer.admin.smsc.SmscManager;

/**
 * @author Artem Snopkov
 */
public class Region2SmscDep {

  private final RegionsManager regionsManager;
  private final SmscManager smscManager;

  public Region2SmscDep(RegionsManager regionsManager, SmscManager smscManager) {
    this.regionsManager = regionsManager;
    this.smscManager = smscManager;
  }

  public void updateRegion(Region region) throws IntegrityException {
    if (smscManager.getSmsc(region.getSmsc()) == null)
      throw new IntegrityException("smsc_not_exist", region.getSmsc());
  }

  public void removeSmsc(String smsc) throws IntegrityException {
    if (!regionsManager.getRegionsBySmsc(smsc).isEmpty())
      throw new IntegrityException("smsc_used_in_regions", smsc);
  }
}
