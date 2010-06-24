package ru.novosoft.smsc.admin.service.daemon;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.protocol.Response;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Демон, управляющий сервисами в HS режиме
 */
public class DaemonHS extends Daemon {

  private final List<String> hosts;

  public DaemonHS(final String host, final int port, final String daemonServicesFolder, List<String> hosts) {
    super(host, port, daemonServicesFolder);
    this.hosts = new ArrayList<String>(hosts);
  }

  protected Map<String, ControlledService> refreshServices() throws AdminException {
    if (super.getStatus() == StatusDisconnected) connect(host, port);
    if (super.getStatus() == StatusConnected) {
      final Response r = runCommand(new CommandListServices());
      if (Response.StatusOk != r.getStatus())
        throw new AdminException("Couldn't list services, nested:" + r.getDataAsString());

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
    throw new AdminException("Unable to connect to demon");
  }

  public void switchOver(final String serviceId) throws AdminException {
    final Response r = runCommand(new CommandSwitchOverService(serviceId));
    if (Response.StatusOk != r.getStatus())
      throw new AdminException("Couldn't switch over callable \"" + serviceId + "\", nested:" + r.getDataAsString());
  }

  public List<String> getHosts() {
    return hosts;
  }
}
