package ru.sibinco.lib.backend.daemon;

import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeManager;

import java.io.File;


/**
 * Created by igork
 * Date: 03.06.2004
 * Time: 21:35:42
 */
public class ServiceInfo
{
  public static final byte STATUS_RUNNING = 0;
  public static final byte STATUS_STARTING = 1;
  public static final byte STATUS_STOPPING = 2;
  public static final byte STATUS_STOPPED = 3;
  public static final byte STATUS_UNKNOWN = 4;

  protected String id = "";
  protected String host = "";
  protected int port = 0;
  protected String args = "";
  protected long pid = 0;
  protected Sme sme = null;
  protected byte status = STATUS_STOPPED;
  private File serviceFolder;
  private boolean autostart;


  public ServiceInfo(Element serviceElement, String serviceHost, SmeManager smeManager, String daemonServicesFolder) throws SibincoException
  {
    host = serviceHost;
    port = Integer.decode(serviceElement.getAttribute("port")).intValue();
    id = serviceElement.getAttribute("id");
    this.autostart = "true".equals(serviceElement.getAttribute("autostart"));
    args = serviceElement.getAttribute("args");

    if (id.equals("")) {
      throw new SibincoException("services name or services system id not specified in response");
    }
    sme = (Sme) smeManager.getSmes().get(id);

    setStatusStr(serviceElement.getAttribute("status"));
    String pidStr = serviceElement.getAttribute("pid");
    this.pid = (pidStr != null && pidStr.length() > 0) ? Long.decode(pidStr).longValue() : 0;
    //? id==folder
    this.serviceFolder = new File(daemonServicesFolder, id);
  }

  private ServiceInfo(String id, String host, int port, String serviceFolder, String args, boolean autostart, long pid, Sme sme, byte status)
  {
    this.host = host;
    this.port = port;
    this.args = args;
    this.autostart = autostart;
    this.pid = pid;
    this.id = id;
    this.sme = sme;
    this.status = status;
    this.serviceFolder = new File(serviceFolder);
  }

  public ServiceInfo(String id, String host, int port, String serviceFolder, String args, boolean autostart, Sme sme, byte status)
  {
    this(id, host, port, serviceFolder, args, autostart, 0, sme, status);
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
    if (pid == 0 && (status == STATUS_RUNNING || status == STATUS_STOPPING))
      status = STATUS_STOPPED;
    if (pid != 0 && (status == STATUS_STARTING || status == STATUS_STOPPED))
      status = STATUS_RUNNING;
  }

  public Sme getSme()
  {
    return sme;
  }

  public byte getStatus()
  {
    return status;
  }

  protected void setStatusStr(String statusStr)
  {
    if ("running".equalsIgnoreCase(statusStr)) {
      this.status = STATUS_RUNNING;
    } else if ("starting".equalsIgnoreCase(statusStr)) {
      this.status = STATUS_STARTING;
    } else if ("stopping".equalsIgnoreCase(statusStr)) {
      this.status = STATUS_STOPPING;
    } else if ("stopped".equalsIgnoreCase(statusStr)) {
      this.status = STATUS_STOPPED;
    } else
      this.status = STATUS_UNKNOWN;
  }

  public String getStatusStr()
  {
    switch (status) {
      case STATUS_RUNNING:
        return "running";
      case STATUS_STARTING:
        return "starting";
      case STATUS_STOPPING:
        return "stopping";
      case STATUS_STOPPED:
        return "stopped";
      case STATUS_UNKNOWN:
        return "unknown";
      default:
        return "unknown";
    }
  }

  public Sme setSme(Sme sme)
  {
    Sme old_sme = this.sme;
    this.sme = sme;
    return old_sme;
  }

  public void setStatus(byte status)
  {
    this.status = status;
  }

  public void setPort(int port)
  {
    this.port = port;
  }

  public void setArgs(String args)
  {
    this.args = args;
  }

  public File getServiceFolder()
  {
    return serviceFolder;
  }

  public boolean isAutostart()
  {
    return autostart;
  }
}
