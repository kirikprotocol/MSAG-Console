package mobi.eyeline.informer.admin.service.daemon;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.protocol.Response;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.File;
import java.util.*;

/**
 * Демон, управляющий сервисами в HS режиме
 */
public class DaemonHS extends Daemon {

  private final List<String> hosts;

  public DaemonHS(final String host, final int port, final File daemonServicesFolder, Collection<String> hosts) {
    super(host, port, daemonServicesFolder);
    this.hosts = new ArrayList<String>(hosts);
  }

  protected Map<String, ControlledService> refreshServices() throws AdminException {
    if (super.getStatus() == StatusDisconnected) connect(host, port);
    if (super.getStatus() == StatusConnected) {
      final Response r = runCommand(new CommandListServices());
      if (Response.StatusOk != r.getStatus())
        throw new DaemonException("error_returned", r.getDataAsString());

      Map<String, ControlledService> services = new HashMap<String, ControlledService>();

      final NodeList list = r.getData().getElementsByTagName("callable");
      for (int i = 0; i < list.getLength(); i++) {
        final Element serviceElement = (Element) list.item(i);
        String id = serviceElement.getAttribute("id");
        boolean autostart = "true".equals(serviceElement.getAttribute("autostart"));
        String args = serviceElement.getAttribute("args");
        String node = serviceElement.getAttribute("node");
        final ControlledService newInfo = new ControlledService(id, node, autostart, new File(daemonServicesFolder, id), args);
        services.put(newInfo.getId(), newInfo);
      }

      return services;
    }
    throw new DaemonException("connection_error");
  }

  public void switchOver(final String serviceId) throws AdminException {
    final Response r = runCommand(new CommandSwitchOverService(serviceId));
    if (Response.StatusOk != r.getStatus())
      throw new DaemonException("error_returned", r.getDataAsString());
  }

  public List<String> getHosts() {
    return hosts;
  }
}
