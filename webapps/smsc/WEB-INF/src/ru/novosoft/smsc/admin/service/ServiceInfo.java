/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:20:22 PM
 */
package ru.novosoft.smsc.admin.service;

public class ServiceInfo
{
  public ServiceInfo(String name, String host, int port, String cmdLine, String[] args, long pid)
  {
    this.name = name;
    this.host = host;
    this.port = port;
    this.cmdLine = cmdLine;
    this.args = args;
    this.pid = pid;
  }

  public ServiceInfo(String name, String host, int port, String cmdLine, String[] args)
  {
    this(name, host, port, cmdLine, args, 0);
  }

  protected String name;
  protected String host;
  protected int port;
  protected String cmdLine;
  protected String[] args;
  protected long pid;

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

  public String[] getArgs()
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
}
