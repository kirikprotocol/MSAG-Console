package ru.novosoft.smsc.admin.daemon;

import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceInfoHS;
import ru.novosoft.smsc.admin.protocol.Response;
import ru.novosoft.smsc.admin.protocol.CommandListServices;

import java.util.Map;

import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 24.03.2006
 * Time: 15:20:05
 * To change this template use File | Settings | File Templates.
 */
public class DaemonHS extends Daemon{
    public DaemonHS(final String host, final int port, final SmeManager smeManager, final String daemonServicesFolder)
    {
      super(host, port, smeManager, daemonServicesFolder);
    }

    protected Map refreshServices(final SmeManager smeManager) throws AdminException
    {
      if (super.getStatus() == StatusDisconnected) connect(host, port);
      if (super.getStatus() == StatusConnected) {
        final Response r = runCommand(new CommandListServices());
        if (Response.StatusOk != r.getStatus())
          throw new AdminException("Couldn't list services, nested:" + r.getDataAsString());

        services.clear();

        final NodeList list = r.getData().getElementsByTagName("service");
        for (int i = 0; i < list.getLength(); i++) {
          final Element serviceElement = (Element) list.item(i);
          final ServiceInfoHS newInfo = new ServiceInfoHS(serviceElement, host, smeManager, daemonServicesFolder);
          services.put(newInfo.getId(), newInfo);
        }
      }
      return services;
    }
}
