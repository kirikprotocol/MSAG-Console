/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:05:54 PM
 */
package ru.novosoft.smsc.admin.daemon;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.protocol.*;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.utli.Proxy;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;


public class Daemon extends Proxy
{
  private Socket socket = null;
  private OutputStream out;
  private InputStream in;
  private CommandWriter writer;
  private ResponseReader reader;
  private Category logger = Category.getInstance(this.getClass().getName());
  private Smsc smsc = null;

  public Daemon(String host, int port, Smsc smsc)
          throws AdminException
  {
    super(host, port);
    this.smsc = smsc;
    connect(host, port);
  }

  /**
   * @return Process ID (PID) of new service
   */
  public long startService(String serviceId)
          throws AdminException
  {
    Response r = runCommand(new CommandStartService(serviceId));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't start service \"" + serviceId + "\", nested:" + r.getDataAsString());

    String pidStr = r.getDataAsString().trim();
    try
    {
      return Long.decode(pidStr).longValue();
    }
    catch (NumberFormatException e)
    {
      throw new AdminException("PID of new service misformatted (" + pidStr + "), nested:" + e.getMessage());
    }
  }

  public void addService(ServiceInfo serviceInfo)
          throws AdminException
  {
    logger.debug("Add service \"" + serviceInfo.getId() + "\" (" + serviceInfo.getHost() + ':'
                 + serviceInfo.getPort() + ")");

    Response r = runCommand(new CommandAddService(serviceInfo));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't add service \"" + serviceInfo.getId() + '/' + serviceInfo.getId()
                               + "\" [" + serviceInfo.getArgs() + "], nested:" + r.getDataAsString());
  }

  public void removeService(String serviceId)
          throws AdminException
  {
    Response r = runCommand(new CommandRemoveService(serviceId));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't remove service \"" + serviceId + "\", nested:" + r.getDataAsString());
  }

  public void shutdownService(String serviceId)
          throws AdminException
  {
    Response r = runCommand(new CommandShutdownService(serviceId));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't shutdown service \"" + serviceId + "\", nested:" + r.getDataAsString());
  }

  public void killService(String serviceId)
          throws AdminException
  {
    Response r = runCommand(new CommandKillService(serviceId));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't kill service \"" + serviceId + "\", nested:" + r.getDataAsString());
  }

  /**
   * Queries demon for services list
   * @return Map: service name -> ServiceInfo
   */
  public Map listServices()
          throws AdminException
  {
    Response r = runCommand(new CommandListServices());
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't list services, nested:" + r.getDataAsString());

    Map result = new HashMap();

    NodeList list = r.getData().getElementsByTagName("service");
    for (int i = 0; i < list.getLength(); i++)
    {
      ServiceInfo newInfo = new ServiceInfo((Element) list.item(i), host, smsc.getSmes());
      result.put(newInfo.getId(), newInfo);
    }

    return result;
  }

  public void setServiceStartupParameters(String serviceId, /*String serviceName, */int port, String args)
          throws AdminException
  {
    Response r = runCommand(new CommandSetServiceStartupParameters(serviceId, /*serviceName, */port, args));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't set service startup parameters \"" + serviceId + "\", nested:" + r.getDataAsString());
  }
}
