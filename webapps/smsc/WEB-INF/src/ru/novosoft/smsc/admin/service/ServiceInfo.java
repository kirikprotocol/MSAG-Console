/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:20:22 PM
 */
package ru.novosoft.smsc.admin.service;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.*;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.smsc.util.StringEncoderDecoder;

public class ServiceInfo
{
  protected String name = "";
  protected String id = "";
  protected String host = "";
  protected int port = 0;
  protected String args = "";
  protected long pid = 0;
  protected Map components = new HashMap();


  public ServiceInfo(Element serviceElement, String serviceHost)
          throws AdminException
  {
    host = serviceHost;
    name = serviceElement.getAttribute("name");
    pid = Long.decode(serviceElement.getAttribute("pid")).longValue();
    port = Integer.decode(serviceElement.getAttribute("port")).intValue();
    id = serviceElement.getAttribute("id");

    args = serviceElement.getAttribute("args");
    if (name.equals("") || id.equals("")) {
      throw new AdminException("service name or service system id not specified in response");
    }
  }

  public ServiceInfo(String name, String id, String host, int port, String args, long pid)
  {
    this.name = name;
    this.host = host;
    this.port = port;
    this.args = args;
    this.pid = pid;
    this.id = id;
  }

  public ServiceInfo(String name, String id, String host, int port, String args)
  {
    this(name, id, host, port, args, 0);
  }


  public String getName()
  {
    return name;
  }

  public String getHost()
  {
    return host;
  }

  public int getPort()
  {
    return port;
  }

  public String getId()
  {
    return id;
  }

  public String getArgs()
  {
    return args;
  }

  public long getPid()
  {
    return pid;
  }

  public void setPid(long pid)
  {
    this.pid = pid;
  }

  public Map getComponents()
  {
    return components;
  }

  public void setComponents(Element response)
  {
    components.clear();
    NodeList list = response.getElementsByTagName("component");
    for (int i = 0; i < list.getLength(); i++) {
      Element compElem = (Element) list.item(i);
      Component c = new Component(compElem);
      components.put(c.getName(), c);
    }
  }
}
