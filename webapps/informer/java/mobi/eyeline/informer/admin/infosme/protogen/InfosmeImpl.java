package mobi.eyeline.informer.admin.infosme.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.infosme.Infosme;

/**
 * @author Aleksandr Khalitov
 */
public class InfosmeImpl implements Infosme {

  private InfosmeClient client;

  public InfosmeImpl(String host, int port) throws AdminException {
    this.client = new InfosmeClient(host, port);
  }
}
