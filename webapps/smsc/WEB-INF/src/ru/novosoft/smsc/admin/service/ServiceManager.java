/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:33:36 PM
 */
package ru.novosoft.smsc.admin.service;

import org.w3c.dom.*;

import java.util.*;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.protocol.Command;
import ru.novosoft.smsc.admin.service.protocol.Response;
import ru.novosoft.smsc.util.config.Config;

public class ServiceManager
{
  protected static ServiceManager manager = null;

  public static ServiceManager getInstance()
  {
    if (manager == null)
      manager = new ServiceManager();
    return manager;
  }


  private Map services = new HashMap();

  protected ServiceManager()
  {
  }

  public synchronized void addService(String serviceName, String host, int port)
  {
    Service s = new Service(host, port);
    services.put(serviceName, s);
  }

  public synchronized Config getServiceConfig(String name)
          throws AdminException
  {
    Service s = (Service) services.get(name);
    if (s != null) {
      Response response = s.runCommand(new Command("get_config"));
      if (response.getStatus() == Response.StatusOk)
        return new Config(response.getData());
    }

    return null;
  }

  public synchronized String getServiceLogs(String name, int startPos, int length)
          throws AdminException
  {
    Service s = (Service) services.get(name);
    if (s != null) {
      Response response = s.runCommand(new Command("get_logs"));
      if (response.getStatus() == Response.StatusOk) {
        NodeList list = response.getData().getDocumentElement().getChildNodes();
        for (int i = 0; i < list.getLength(); i++) {
          if (list.item(i).getNodeType() == Node.TEXT_NODE) {
            return list.item(i).getNodeValue();
          }
        }
      }
    }

    return null;
  }

  public synchronized Map getServiceMonitoringData(String name)
          throws AdminException
  {
    Map result = null;
    Service s = (Service) services.get(name);
    if (s != null) {
      Response response = s.runCommand(new Command("get_monitoring"));
      if (response.getStatus() == Response.StatusOk) {
        result = new HashMap();
        NodeList list = response.getData().getDocumentElement().getChildNodes();
        for (int i = 0; i < list.getLength(); i++) {
          if (list.item(i).getNodeType() == Node.ELEMENT_NODE) {
            Element e = (Element) list.item(i);
            NamedNodeMap attrs = e.getAttributes();
            for (int j = 0; j < attrs.getLength(); j++) {
              Node a = attrs.item(j);
              result.put(a.getNodeName(), Integer.decode(a.getNodeValue()));
            }
          }
        }
      }
    }

    return result;
  }

  public synchronized boolean shutdownService(String name)
          throws AdminException
  {
    Service s = (Service) services.get(name);
    if (s != null) {
      Response response = s.runCommand(new Command("shutdown"));
      return response.getStatus() == Response.StatusOk;
    }
    throw new AdminException("Unknown service \"" + name + '"');
  }

  public synchronized Set getServiceNames()
  {
    return services.keySet();
  }
}
