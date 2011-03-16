package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.Visitor;
import org.apache.log4j.Logger;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class SiebelProvider {

  private static final Logger logger = Logger.getLogger("SIEBEL");

  private final SiebelContext context;
  private SiebelSettings settings;

  private SiebelDataProvider dataProvider;
  private SiebelManager manager;
  private SiebelFinalStateListener changeListener;

  public SiebelProvider(SiebelContext context, SiebelSettings settings, File workDir) throws AdminException, InitException {
    dataProvider = new SiebelDataProviderImpl();
    manager = new SiebelManager(dataProvider, context);
    changeListener = new SiebelFinalStateListener(manager, context, workDir, settings.getStatsPeriod());
    context.getDeliveryChangesDetector().addListener(changeListener);

    manager.start(settings.getUser(), settings);
    changeListener.start();

    this.context = context;
    this.settings = settings;
  }

  public void shutdown() {
    context.getDeliveryChangesDetector().removeListener(changeListener);
    manager.stop();
    changeListener.shutdown();
    dataProvider.shutdown();
  }

  public void checkSettings(SiebelSettings settings) throws AdminException {
    settings.validate();
    manager.checkProperties(settings);
  }

  public boolean updateSettings(SiebelSettings settings) throws AdminException {

    String u = settings.getUser();

    SiebelSettings old = this.settings;
    if (!old.getUser().equals(settings.getUser())) {
      DeliveryFilter filter = new DeliveryFilter();
      filter.setUserIdFilter(old.getUser());
      final boolean[] notExist = new boolean[]{true};
      context.getDeliveries(u, filter, new Visitor<Delivery>() {
        public boolean visit(Delivery value) throws AdminException {
          if (value.getStatus() != DeliveryStatus.Finished && value.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID) != null) {
            notExist[0] = false;
            return false;
          }
          return true;
        }
      });
      if (!notExist[0]) {
        throw new SiebelException("can_not_change_user", old.getUser());
      }
    }

    boolean siebelStarted = false;
    try {
      changeListener.externalLock();
      changeListener.setPeriodSec(settings.getStatsPeriod());
      manager.stop();
      try {
        SiebelSettings _p = new SiebelSettings(settings);
        manager.start(u, _p);
        if(!changeListener.isStarted()) {
          changeListener.start();
        }
        siebelStarted = true;
      } catch (Exception e) {
        logger.error("Applying of new properties has failed. Siebel is down.", e);
      }
    } finally {
      changeListener.externalUnlock();
    }
    return siebelStarted;
  }
}
