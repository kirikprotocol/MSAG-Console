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

public class ServiceInfo
{
  public ServiceInfo(Element serviceElement, String serviceHost)
          throws AdminException
  {
    host = serviceHost;
    name = serviceElement.getAttribute("name");
    pid = Long.decode(serviceElement.getAttribute("pid")).longValue();
    cmdLine = serviceElement.getAttribute("command_line");
    configFileName = serviceElement.getAttribute("config");
    port = Integer.decode(serviceElement.getAttribute("port")).intValue();

    NodeList childs = serviceElement.getElementsByTagName("arg");
    args.setSize(childs.getLength());
    for (int j = 0; j < childs.getLength(); j++) {
      Element argElem = (Element) childs.item(j);
      int num = Integer.decode(argElem.getAttribute("num")).intValue();
      args.set(num, Utils.getNodeText(argElem));
    }
    if (this.name.equals("") || this.cmdLine.equals("")) {
      throw new AdminException("service name or command line not specified in response");
    }
  }

  public ServiceInfo(String name, String host, int port, String cmdLine, String configFileName, Vector args, long pid)
  {
    this.name = name;
    this.host = host;
    this.port = port;
    this.cmdLine = cmdLine;
    this.configFileName = configFileName;
    this.args = args;
    this.pid = pid;
  }

  public ServiceInfo(String name, String host, int port, String cmdLine, String configFileName, Vector args)
  {
    this(name, host, port, cmdLine, configFileName, args, 0);
  }

  protected String name = "";
  protected String host = "";
  protected int port = 0;
  protected String cmdLine = "";

  protected String configFileName = "config.xml";
  protected Vector args = new Vector();
  protected long pid = 0;
  protected Map components = new HashMap();

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

  public String getCmdLine()
  {
    return cmdLine;
  }

  public String getConfigFileName()
  {
    return configFileName;
  }

  public Vector getArgs()
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
