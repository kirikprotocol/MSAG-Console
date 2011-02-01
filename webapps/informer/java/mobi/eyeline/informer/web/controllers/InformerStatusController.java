package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.Collections;
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

}
