package ru.sibinco.lib.backend.service;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.service.Component;

import java.io.File;
import java.util.Map;
import java.util.HashMap;


/**
 * Created by igork Date: 03.06.2004 Time: 21:35:42
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
  protected String args = "";
  protected long pid = 0;
  protected Sme sme = null;
  protected byte status = STATUS_STOPPED;
  private File serviceFolder;
  private boolean autostart;
   protected Map components = new HashMap();


  public ServiceInfo(final Element serviceElement, final String serviceHost, final SmeManager smeManager, final String daemonServicesFolder)
      throws SibincoException
  {
    host = serviceHost;
    id = serviceElement.getAttribute("id");
    this.autostart = "true".equals(serviceElement.getAttribute("autostart"));
    args = serviceElement.getAttribute("args");

    if ("".equals(id)) {
      throw new SibincoException("services name or services system id not specified in response");
    }
    sme = (Sme) smeManager.getSmes().get(id);

    setStatusStr(serviceElement.getAttribute("status"));
    final String pidStr = serviceElement.getAttribute("pid");
    this.pid = null != pidStr && 0 < pidStr.length() ? Long.decode(pidStr).longValue() : 0;
    //? id==folder
    this.serviceFolder = new File(daemonServicesFolder, id);
  }

  private ServiceInfo(final String id, final String host, final String serviceFolder, final String args, final boolean autostart, final long pid,
                      final Sme sme, final byte status)
  {
    this.host = host;
    this.args = args;
    this.autostart = autostart;
    this.pid = pid;
    this.id = id;
    this.sme = sme;
    this.status = status;
    this.serviceFolder = new File(serviceFolder);
  }

  public ServiceInfo(final String id, final String host, final String serviceFolder, final String args, final boolean autostart, final Sme sme,
                     final byte status)
  {
    this(id, host, serviceFolder, args, autostart, 0, sme, status);
  }


  public String getHost()
  {
    return host;
  }

  public String getId()
  {
    return id;
  }

  public String getArgs()
  {
    return args;
  }

  public Map getComponents()
  {
    return components;
  }
  public void setComponents(final Element response)
   {
     components.clear();
     final NodeList list = response.getElementsByTagName("component");
     for (int i = 0; i < list.getLength(); i++) {
       final Element compElem = (Element) list.item(i);
       final Component c = new Component(compElem);
       components.put(c.getName(), c);
     }
   }
  
  public long getPid()
  {
    return pid;
  }

  public void setPid(final long pid)
  {
    this.pid = pid;
    if (0 == pid && (STATUS_RUNNING == status || STATUS_STOPPING == status))
      status = STATUS_STOPPED;
    if (0 != pid && (STATUS_STARTING == status || STATUS_STOPPED == status))
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

  protected void setStatusStr(final String statusStr)
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

  public Sme setSme(final Sme sme)
  {
    final Sme old_sme = this.sme;
    this.sme = sme;
    return old_sme;
  }

  public void setStatus(final byte status)
  {
    this.status = status;
  }

  public void setArgs(final String args)
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
