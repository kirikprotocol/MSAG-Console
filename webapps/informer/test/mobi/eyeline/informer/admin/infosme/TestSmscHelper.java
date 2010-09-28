package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class TestSmscHelper extends TestConfigHelper{

  private String defaultSmsc;


  public synchronized void setDefaultSmsc(String smscId) throws AdminException {
    if(!smscId.contains(smscId)) {
      throw new InfosmeException("interaction_error","");
    }
    defaultSmsc = smscId;
  }
}
