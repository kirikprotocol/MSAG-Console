/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:35:08 PM
 */
package ru.novosoft.smsc.admin.service;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.util.*;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.protocol.*;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.Utils;

public class Service extends Proxy
{
  ServiceInfo info = null;

  public Service(ServiceInfo info)
          throws AdminException
  {
    super(info.getHost(), info.getPort());
    this.info = info;
  }

  public ServiceInfo getInfo()
  {
    return info;
  }

  public Config getConfig()
          throws AdminException
  {
    if (info.getPid() == 0)
      throw new AdminException("Service \"" + info.getName() + "\"not started");

    Response response = runCommand(new CommandGetServiceConfig());
    if (response.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't get config, nested:" + response.getDataAsString());
    return new Config(response.getData());
  }

  public String getLogs(long startpos, long length)
          throws AdminException
  {
    if (info.getPid() == 0)
      throw new AdminException("Service \"" + info.getName() + "\"not started");

    Response response = runCommand(new CommandGetServiceLogs(startpos, length));
    if (response.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't get logs, nested:" + response.getDataAsString());
    return Utils.getNodeText(response.getData().getDocumentElement());
  }

  public Map getMonitoringData()
          throws AdminException
  {
    if (info.getPid() == 0)
      throw new AdminException("Service \"" + info.getName() + "\"not started");

    Response response = runCommand(new CommandGetServiceMonitoringData());
    if (response.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't get monitoring data, nested:" + response.getDataAsString());

    Map result = new HashMap();
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
    return result;
  }

  public void setConfig(Config config)
          throws AdminException
  {
    if (info.getPid() == 0)
      throw new AdminException("Service \"" + info.getName() + "\"not started");

    Response response = runCommand(new CommandSetServiceConfig(config));
    if (response.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't set config, nested:" + response.getDataAsString());
  }

  public void shutdown()
          throws AdminException
  {
    if (info.getPid() == 0)
      throw new AdminException("Service \"" + info.getName() + "\"not started");

    Response response = runCommand(new CommandShutdownService());
    if (response.getStatus() != Response.StatusOk)
      throw new AdminException("Couldn't shutdown service, nested:" + response.getDataAsString());

    info.setPid(0);
  }
}
