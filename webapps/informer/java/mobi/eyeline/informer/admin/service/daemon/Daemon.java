/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:05:54 PM
 */
package mobi.eyeline.informer.admin.service.daemon;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.protocol.Proxy;
import mobi.eyeline.informer.admin.protocol.Response;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.File;
import java.util.*;


/**
 * Демон, управляющий сервисами в Single режиме.
 */
public class Daemon extends Proxy {

  protected final File daemonServicesFolder;

  public Daemon(final String host, final int port, final File daemonServicesFolder) {
    super(host, port);
    this.daemonServicesFolder = daemonServicesFolder;
    try {
      connect(host, port);
      refreshServices();
    } catch (AdminException e) {
      e.printStackTrace();
      logger.error("Couldn't add daemon \"" + host + "\"", e);
    }
  }

  public ControlledService getService(String serviceId) throws AdminException {
    return refreshServices().get(serviceId);
  }

  protected synchronized Map<String, ControlledService> refreshServices() throws AdminException {
    if (super.getStatus() == StatusDisconnected) connect(host, port);
    if (super.getStatus() == StatusConnected) {
      final Response r = runCommand(new CommandListServices());
      if (Response.StatusOk != r.getStatus())
        throw new DaemonException("error_returned", r.getDataAsString());

      Map<String, ControlledService> services = new HashMap<String, ControlledService>();

      final NodeList list = r.getData().getElementsByTagName("service");
      for (int i = 0; i < list.getLength(); i++) {
        final Element serviceElement = (Element) list.item(i);
        String id = serviceElement.getAttribute("id");
        boolean autostart = "true".equals(serviceElement.getAttribute("autostart"));
        String args = serviceElement.getAttribute("args");
        String status = serviceElement.getAttribute("status");

        String host = status.equals("running") ? this.host : null;

        final ControlledService newInfo = new ControlledService(id, host, autostart, new File(daemonServicesFolder, id), args);
        services.put(newInfo.getId(), newInfo);
      }

      return services;
    }
    throw new DaemonException("connection_error");
  }

  /**
   * Запускает сервис с указанным Id
   *
   * @param serviceId идентификатор сервиса
   * @throws AdminException ошибка выполнения команды
   */
  public void startService(final String serviceId) throws AdminException {
    final Response r = runCommand(new CommandStartService(serviceId));
    if (Response.StatusOk != r.getStatus())
      throw new DaemonException("error_returned", r.getDataAsString());
  }

  /**
   * Добавляет новый администрируемый сервис
   *
   * @param id        идентификатор сервиса
   * @param args      список аргументов
   * @param status    статус
   * @param autostart надо ли автоматически запускать или нет
   * @throws AdminException ошибка выполнения команды
   */
  public void addService(String id, String args, String status, boolean autostart) throws AdminException {
    final Response r = runCommand(new CommandAddService(id, args, status, autostart));
    if (Response.StatusOk != r.getStatus())
      throw new DaemonException("error_returned", r.getDataAsString());
  }

  /**
   * Удаляет сервис с заданным идентификатором
   *
   * @param serviceId идентификатор сервиса
   * @throws AdminException ошибка выполнения команды
   */
  public void removeService(final String serviceId) throws AdminException {
    final Response r = runCommand(new CommandRemoveService(serviceId));
    if (Response.StatusOk != r.getStatus())
      throw new DaemonException("error_returned", r.getDataAsString());
  }

  /**
   * Останавливает сервис с заданным идентификатором
   *
   * @param serviceId идентификатор сервиса
   * @throws AdminException ошибка выполнения команды
   */
  public void shutdownService(final String serviceId) throws AdminException {
    final Response r = runCommand(new CommandShutdownService(serviceId));
    if (Response.StatusOk != r.getStatus()) {
      throw new DaemonException("error_returned", r.getDataAsString());
    }
  }

  public void killService(final String serviceId) throws AdminException {
    final Response r = runCommand(new CommandKillService(serviceId));
    if (Response.StatusOk != r.getStatus()) {
      throw new DaemonException("error_returned", r.getDataAsString());
    }
  }

  public Collection<String> getServiceIds() throws AdminException {
    Map<String, ControlledService> services = refreshServices();
    if (0 == services.size())
      refreshServices();
    return services.keySet();
  }

  public boolean isContainsService(final String serviceId) throws AdminException {
    Map<String, ControlledService> services = refreshServices();
    return services.containsKey(serviceId);
  }

  public void removeAllServices() throws AdminException {
    Map<String, ControlledService> services = refreshServices();
    final Set<String> serviceIds = new HashSet<String>(services.keySet());
    for (String serviceId : serviceIds)
      removeService(serviceId);
  }

}
