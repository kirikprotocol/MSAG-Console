package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

/**
 * @author Aleksandr Khalitov
 */
public class ArchiveDaemonSettings {

  private static final ValidationHelper vh = new ValidationHelper(ArchiveDaemonSettings.class);

  private String dcpHost;

  private int dcpPort;

  public String getDcpHost() {
    return dcpHost;
  }

  public void setDcpHost(String dcpHost) {
    this.dcpHost = dcpHost;
  }

  public int getDcpPort() {
    return dcpPort;
  }

  public void setDcpPort(int dcpPort) {
    this.dcpPort = dcpPort;
  }

  void validate() throws AdminException {
    vh.checkNotEmpty("dcpHost", dcpHost);
    vh.checkPort("dcpPort", dcpPort);
  }
}
