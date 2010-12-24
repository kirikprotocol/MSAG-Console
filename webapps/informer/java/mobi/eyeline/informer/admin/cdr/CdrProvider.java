package mobi.eyeline.informer.admin.cdr;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class CdrProvider {

  private CdrDaemon daemon;
  private final CdrProviderContext context;

  public CdrProvider (CdrProviderContext context, CdrSettings settings, File workDir, FileSystem fs) throws InitException {
    this.daemon = new CdrDaemon(workDir, new File(settings.getCdrDir()), fs, context);
    this.context = context;
    this.context.getDeliveryChangesDetector().addListener(daemon);
    this.daemon.start();
  }

  public void shutdown() {
    this.context.getDeliveryChangesDetector().removeListener(daemon);
    try {
      daemon.stop();
    } catch (Exception ignored) {
    }
  }

  public void updateSettings(CdrSettings settings) throws AdminException {
    settings.validate();
    daemon.setCdrOutputDir(new File(settings.getCdrDir()));
  }

}
