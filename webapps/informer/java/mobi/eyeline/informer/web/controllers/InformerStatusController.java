package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * Котроллер для остановки/запуска сервисов
 *
 * @author Aleksandr Khalitov
 */

public class InformerStatusController extends InformerController {

  private String informerOnlineHost;
  private List<String> informerHosts;
  private String informerSwitchTo;

  private String ftpServerOnlineHost;
  private List<String> ftpServerHosts;
  private String ftpServerSwitchTo;

  private String archiveDaemonOnlineHost;
  private List<String> archiveDaemonHosts;
  private String archiveDaemonSwitchTo;

  private String pvssOnlineHost;
  private List<String> pvssHosts;
  private String pvssSwitchTo;

  public InformerStatusController() {
    try {
      reload();
    } catch (AdminException e) {
      addError(e);
    }
  }

  private void reload() throws AdminException {
    Configuration c = getConfig();
    this.informerOnlineHost = c.getInformerOnlineHost();
    this.informerHosts = c.getInformerHosts();
    if (c.isFtpServerDeployed()) {
      this.ftpServerHosts = c.getFtpServerHosts();
      this.ftpServerOnlineHost = c.getFtpServerOnlineHost();
    } else {
      ftpServerHosts = Collections.emptyList();
    }
    if (c.isArchiveDaemonDeployed()) {
      this.archiveDaemonHosts = c.getArchiveDaemonHosts();
      this.archiveDaemonOnlineHost = c.getArchiveDaemonOnlineHost();
    } else {
      archiveDaemonHosts = Collections.emptyList();
    }
    if (c.isPVSSDeployed()) {
      this.pvssHosts = c.getPVSSHosts();
      this.pvssOnlineHost = c.getPVSSOnlineHost();
    } else {
      pvssHosts = Collections.emptyList();
    }
  }

  public String startInformer() {
    if (informerOnlineHost == null) {
      try {
        getConfig().startInformer(getUserName());
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String stopInformer() {
    if (informerOnlineHost != null) {
      try {
        getConfig().stopInformer(getUserName());
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String switchInformerHost() {
    try {
      getConfig().switchInformer(informerSwitchTo, getUserName());
      reload();
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String getInformerOnlineHost() {
    return informerOnlineHost;
  }

  public List<SelectItem> getInformerHosts() {
    List<SelectItem> items = new ArrayList<SelectItem>(informerHosts.size());
    for (String host : informerHosts) {
      if (informerOnlineHost == null || !host.equals(informerOnlineHost))
        items.add(new SelectItem(host, host));
    }
    Collections.sort(items, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return items;
  }

  public boolean isInformerSwitchAllowed() {
    return informerHosts != null && informerHosts.size() > 1;
  }

  public String getInformerSwitchTo() {
    return informerSwitchTo;
  }

  public void setInformerSwitchTo(String informerSwitchTo) {
    this.informerSwitchTo = informerSwitchTo;
  }

  public boolean isFtpServerDeployed() {
    return getConfig().isFtpServerDeployed();
  }

  public boolean isArchiveDaemonDeployed() {
    return getConfig().isArchiveDaemonDeployed();
  }

  public boolean isPvssDeployed() {
    return getConfig().isPVSSDeployed();
  }

  public String startFtpServer() {
    if (ftpServerOnlineHost == null) {
      try {
        getConfig().startFtpServer(getUserName());
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String startArchiveDaemon() {
    if (archiveDaemonOnlineHost == null) {
      try {
        getConfig().startArchiveDaemon(getUserName());
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }
  public String startPvss() {
    if (pvssOnlineHost == null) {
      try {
        getConfig().startPVSS(getUserName());
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String stopFtpServer() {
    if (ftpServerOnlineHost != null) {
      try {
        getConfig().stopFtpServer(getUserName());
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String switchFtpServerHost() {
    try {
      getConfig().switchFtpServer(ftpServerSwitchTo, getUserName());
      reload();
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String getFtpServerOnlineHost() {
    return ftpServerOnlineHost;
  }

  public List<SelectItem> getFtpServerHosts() {
    List<SelectItem> items = new ArrayList<SelectItem>(ftpServerHosts.size());
    for (String host : ftpServerHosts) {
      if (ftpServerOnlineHost == null || !host.equals(ftpServerOnlineHost))
        items.add(new SelectItem(host, host));
    }
    Collections.sort(items, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return items;
  }

  public boolean isFtpServerSwitchAllowed() {
    return ftpServerHosts != null && ftpServerHosts.size() > 1;
  }

  public String getFtpServerSwitchTo() {
    return ftpServerSwitchTo;
  }

  public void setFtpServerSwitchTo(String ftpServerSwitchTo) {
    this.ftpServerSwitchTo = ftpServerSwitchTo;
  }

  public String stopArchiveDaemon() {
    if (archiveDaemonOnlineHost != null) {
      try {
        getConfig().stopArchiveDaemon(getUserName());
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }
  public String stopPvss() {
    if (pvssOnlineHost != null) {
      try {
        getConfig().stopPvss(getUserName());
        reload();
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String switchArchiveDaemonHost() {
    try {
      getConfig().switchArchiveDaemon(archiveDaemonSwitchTo, getUserName());
      reload();
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }
  public String switchPvssHost() {
    try {
      getConfig().switchPvss(pvssSwitchTo, getUserName());
      reload();
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String getArchiveDaemonOnlineHost() {
    return archiveDaemonOnlineHost;
  }
  public String getPvssOnlineHost() {
    return pvssOnlineHost;
  }

  public List<SelectItem> getArchiveDaemonHosts() {
    List<SelectItem> items = new ArrayList<SelectItem>(archiveDaemonHosts.size());
    for (String host : archiveDaemonHosts) {
      if (archiveDaemonOnlineHost == null || !host.equals(archiveDaemonOnlineHost))
        items.add(new SelectItem(host, host));
    }
    Collections.sort(items, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return items;
  }

  public boolean isArchiveDaemonSwitchAllowed() {
    return archiveDaemonHosts != null && archiveDaemonHosts.size() > 1;
  }

  public String getArchiveDaemonSwitchTo() {
    return archiveDaemonSwitchTo;
  }

  public void setArchiveDaemonSwitchTo(String archiveDaemonSwitchTo) {
    this.archiveDaemonSwitchTo = archiveDaemonSwitchTo;
  }
  public List<SelectItem> getPvssHosts() {
    List<SelectItem> items = new ArrayList<SelectItem>(pvssHosts.size());
    for (String host : pvssHosts) {
      if (pvssOnlineHost == null || !host.equals(pvssOnlineHost))
        items.add(new SelectItem(host, host));
    }
    Collections.sort(items, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return items;
  }

  public boolean isPvssSwitchAllowed() {
    return pvssHosts != null && pvssHosts.size() > 1;
  }

  public String getPvssSwitchTo() {
    return pvssSwitchTo;
  }

  public void setPvssSwitchTo(String pvssSwitchTo) {
    this.pvssSwitchTo = pvssSwitchTo;
  }

}
