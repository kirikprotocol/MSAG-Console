/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:05:54 PM
 */
package ru.novosoft.smsc.admin.daemon;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;

import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.net.Socket;
import java.util.*;

import ru.novosoft.smsc.admin.protocol.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.util.xml.Utils;

public class Daemon extends Proxy
{
  private Socket socket = null;
  private OutputStream out;
  private InputStream in;
  private CommandWriter writer;
  private ResponseReader reader;
  private Category logger = Category.getInstance(this.getClass().getName());

  public Daemon(String host, int port)
          throws AdminException
  {
    super(host, port);
    connect(host, port);
  }

  /**
   * @return Process ID (PID) of new service
   */
  public long startService(String serviceName)
          throws AdminException
  {
    if (getStatus() != StatusConnected)
      connect(host, port);

    Response r = runCommand(new CommandStartService(serviceName));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't start service \"" + serviceName + "\", nested:" + r.getDataAsString());

    String pidStr = r.getDataAsString().trim();
    try {
      return Long.decode(pidStr).longValue();
    } catch (NumberFormatException e) {
      throw new AdminException("PID of new service misformatted (" + pidStr + "), nested:" + e.getMessage());
    }
  }

  public void addService(ServiceInfo serviceInfo)
    throws AdminException
  {
    logger.debug("Add service \"" + serviceInfo.getName() + "\" (" + serviceInfo.getHost() + ':'
                 + serviceInfo.getPort() + ")");

    if (getStatus() != StatusConnected)
      connect(host, port);

    logger.debug("checkpoint 1");
    Response r = runCommand(new CommandAddService(serviceInfo));
    logger.debug("checkpoint 2");
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't add service \"" + serviceInfo.getName()
                               + "\" [" + serviceInfo.getCmdLine() + " "
                               + serviceInfo.getArgs() +"], nested:" + r.getDataAsString());
  }

  public void removeService(String serviceName)
    throws AdminException
  {
    if (getStatus() != StatusConnected)
      connect(host, port);

    Response r = runCommand(new CommandRemoveService(serviceName));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't remove service \"" + serviceName + "\", nested:" + r.getDataAsString());
  }

  public void killService(String serviceName)
          throws AdminException
  {
    if (getStatus() != StatusConnected)
      connect(host, port);

    Response r = runCommand(new CommandKillService(serviceName));
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't kill service \"" + serviceName + "\", nested:" + r.getDataAsString());
  }

  /**
   * Queries demon for services list
   * @return Map: service name -> ServiceInfo
   */
  public Map listServices()
          throws AdminException
  {
    if (getStatus() != StatusConnected)
      connect(host, port);

    Response r = runCommand(new CommandListServices());
    if (r.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't list services, nested:" + r.getDataAsString());

    Map result = new HashMap();

    NodeList list = r.getData().getElementsByTagName("service");
    for (int i=0; i<list.getLength(); i++)
    {
      String serviceName = "";
      String serviceCommandLine = "";
      long servicePid = 0;
      int servicePort = 0;
      Map argsMap = new HashMap();

      Node n = list.item(i);
      NodeList childs = n.getChildNodes();
      for (int j=0; j<childs.getLength(); j++)
      {
        Node c = childs.item(j);
        if (c.getNodeType() == Node.ELEMENT_NODE)
        {
          String name = c.getNodeName();
          String value = Utils.getNodeText(c);
          if (name.equals("name"))
            serviceName = value;
          else if (name.equals("command_line"))
            serviceCommandLine = value;
          else if (name.equals("pid"))
          {
            try {
              servicePid = Long.decode(value).longValue();
            } catch (NumberFormatException e) {
              throw new AdminException("misformatted pid \""+value+"\" in response, nested:" + e.getMessage());
            }
          }
          else if (name.equals("port"))
          {
            try {
              servicePort = Integer.decode(value).intValue();
            } catch (NumberFormatException e) {
              throw new AdminException("misformatted port \""+value+"\" in response, nested:" + e.getMessage());
            }
          }
          else if (name.equals("arg"))
          {
            Integer argNum = Integer.decode(c.getAttributes().getNamedItem("num").getNodeValue());
            argsMap.put(argNum, value);
          }
        }
      }
      if (serviceName.equals("") || serviceCommandLine.equals(""))
      {
        throw new AdminException("service name or command line not specified in response");
      }
      String[] args = new String[argsMap.size()];
      for (int j=0; j<argsMap.size(); j++)
      {
        args[j] = (String)argsMap.get(new Integer(j));
        if (args[j] == null)
          args[j] = "";
      }
      result.put(serviceName, new ServiceInfo(serviceName, this.host, servicePort, serviceCommandLine, args, servicePid));
    }

    return result;
  }
}
