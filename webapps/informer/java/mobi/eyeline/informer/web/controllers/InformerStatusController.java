package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * Котроллер для остановки/запуска сервисов
 * @author Aleksandr Khalitov
 */

public class InformerStatusController extends InformerController{

  private String onlineHost;
  private List<String> hosts;
  private String switchTo;

  public InformerStatusController() {
    try{
      reload();
    }catch (AdminException e) {
      addError(e);
    }
  }

  private void reload() throws AdminException{
    Configuration c = getConfig();
    this.onlineHost = c.getInformerOnlineHost();
    this.hosts = c.getInformerHosts();
  }

  public String start() {
    if(onlineHost == null) {
      try{
        getConfig().startInformer(getUserName());
        reload();
      }catch (AdminException e){
        addError(e);
      }
    }
    return null;
  }

  public String stop() {
    if(onlineHost != null) {
      try{
        getConfig().stopInformer(getUserName());
        reload();
      }catch (AdminException e){
        addError(e);
      }
    }
    return null;
  }

  public String switchHost() {
      try{
        getConfig().switchInformer(switchTo, getUserName());
        reload();
      }catch (AdminException e){
        addError(e);
      }
    return null;
  }

  public String getOnlineHost() {
    return onlineHost;
  }

  public List<SelectItem> getHosts() {
    List<SelectItem> items = new ArrayList<SelectItem>(hosts.size());
    for (String host : hosts) {
      if (onlineHost == null || !host.equals(onlineHost))
        items.add(new SelectItem(host, host));
    }
    return items;
  }

  public boolean isSwitchAllowed() {
    return hosts != null && hosts.size() > 1;
  }

  public String getSwitchTo() {
    return switchTo;
  }

  public void setSwitchTo(String switchTo) {
    this.switchTo = switchTo;
  }
}
