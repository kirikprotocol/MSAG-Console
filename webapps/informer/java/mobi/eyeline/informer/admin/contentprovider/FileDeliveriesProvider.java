package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class FileDeliveriesProvider {

  private ContentProviderDaemon daemon;
  private ContentProviderContext context;

  public FileDeliveriesProvider(ContentProviderContext context, File baseDir, File workDir, int periodSec) throws AdminException {
    daemon = new ContentProviderDaemon(context, baseDir, workDir, periodSec);
    this.context = context;
    context.getDeliveryChangesDetector().addListener(daemon);
    daemon.start();
  }

  public void shutdown() {
    context.getDeliveryChangesDetector().removeListener(daemon);
    try {
      daemon.stop();
    } catch (AdminException ignored) {
    }
  }

  public void verifyConnection(User u, UserCPsettings ucps) throws AdminException {
    daemon.verifyConnection(u, ucps);
  }
}
