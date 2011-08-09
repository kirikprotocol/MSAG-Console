package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Logger;

/**
 * author: Aleksandr Khalitov
 */
public class TestSmppGW implements SmppGW{

  private static final Logger logger = Logger.getLogger(TestSmppGW.class);

  @Override
  public void updateConfig() throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Smpp GW has been updated");
    }
  }

  @Override
  public void shutdown() {

  }
}
