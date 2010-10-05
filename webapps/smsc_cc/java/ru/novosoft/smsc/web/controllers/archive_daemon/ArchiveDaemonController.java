package ru.novosoft.smsc.web.controllers.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.*;


public class ArchiveDaemonController extends SettingsMController<ArchiveDaemonSettings> {

  private boolean initError;
  private String switchToHost;
  ArchiveDaemonSettings config;


  private DynamicTableModel locationSources;
  private DynamicTableModel indexatorSmeAddrChunkSizes;


  protected  DynamicTableModel buildModel(Set entrySet) {
    DynamicTableModel dm = new DynamicTableModel();
    for (Map.Entry e : (Set<Map.Entry>)entrySet) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("key", e.getKey());
      row.setValue("value", e.getValue());
      dm.addRow(row);
    }
    return dm;
  }

  @Override
  protected void init() throws AdminException {
    super.init();    //To change body of overridden methods use File | Settings | File Templates.
    config=getSettings();
    switchToHost = WebContext.getInstance().getArchiveDaemonManager().getDaemonOnlineHost();
    locationSources = buildModel(config.getLocationsSources().entrySet());
    indexatorSmeAddrChunkSizes = buildModel(config.getIndexatorSmeAddrChunkSizes().entrySet());
  }

  public ArchiveDaemonController() {
    super(WebContext.getInstance().getArchiveDaemonManager());
    try {
      init();

    } catch (AdminException e) {
      addError(e);
      initError = true;
      return;
    }
    checkChanges();
  }

  public boolean isInitError() {
    return initError;
  }

  private void checkChanges() {
    if (isSettingsChanged())
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "smsc.submit.hint");
  }


  public String submit() {
    try {

      Map<String, String> ls = new HashMap<String, String>(locationSources.getRowCount());
      for (DynamicTableRow e : locationSources.getRows()) {
        String key = (String) e.getValue("key");
        if (key == null || key.length() == 0) {
          addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archiveDaemon.location.source.key.empty");
          return null;
        }
        ls.put(key, (String) e.getValue("value"));
      }
      config.setLocationsSources(ls);

      Map<String, Integer> ic = new HashMap<String, Integer>(indexatorSmeAddrChunkSizes.getRowCount());
      for (DynamicTableRow row : indexatorSmeAddrChunkSizes.getRows()) {
        String key = (String) row.getValue("key");
        if (key == null || key.length() == 0) {
          addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archiveDaemon.indexatorSmeAddrChunkSizes.source.key.empty");
          return null;
        }
        try {
          Integer val  = Integer.valueOf((String)row.getValue("value"));
          if(val<=0) {
            addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archiveDaemon.indexatorSmeAddrChunkSizes.source.val.notpositive");
          }
          ic.put(key,val);
        }
        catch (NumberFormatException ee) {
          addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archiveDaemon.indexer.value.nan");
          return null;
        }
      }
      config.setIndexatorSmeAddrChunkSizes(ic);


      setSettings(config);
      submitSettings();

    }
    catch (AdminException e) {
      addError(e);
      return null;
    }
    return "INDEX";
  }

  public String reset() {
    try {
      resetSettings();
    } catch (AdminException e) {
      addError(e);
    }
    return "ARCHIVEDAEMON";
  }

  public List<String> getAvailableHosts() throws AdminException {
    List<String> ret = new ArrayList<String>();
    String daemonOnlineHost = WebContext.getInstance().getArchiveDaemonManager().getDaemonOnlineHost();
    for(String hName : WebContext.getInstance().getArchiveDaemonManager().getDaemonHosts()) {
      if(!hName.equals(daemonOnlineHost)) ret.add(hName);
    }
    return ret;
  }
  public List<SelectItem> getHostsItems() throws AdminException {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    String daemonOnlineHost = WebContext.getInstance().getArchiveDaemonManager().getDaemonOnlineHost();
    for(String hName : WebContext.getInstance().getArchiveDaemonManager().getDaemonHosts()) {
      ret.add(new SelectItem(hName));
    }
    return ret;
  }

  public String getOnlineHost() throws AdminException {
    return WebContext.getInstance().getArchiveDaemonManager().getDaemonOnlineHost();
  }


  public void setSwitchToHost(String switchToHost) {
    this.switchToHost = switchToHost;
  }

  public String getSwitchToHost() {
    return switchToHost;
  }

  public String switchHost() throws AdminException {
    
     if(switchToHost!=null) {
       WebContext.getInstance().getArchiveDaemonManager().switchDaemon(switchToHost);
     }
     return null;
  }


  public String start() throws AdminException {

    if(WebContext.getInstance().getArchiveDaemonManager().getDaemonOnlineHost()!=null){
      return null;
    }
    if(getAvailableHosts().size()==1)  {
      WebContext.getInstance().getArchiveDaemonManager().startDaemon();
    }
    else {
      switchHost();
    }
    return null;
  }

  public boolean isShowSwitchLink() throws AdminException {
    return
    (WebContext.getInstance().getArchiveDaemonManager().getDaemonOnlineHost()!=null) 
    &&
    (getAvailableHosts().size()>0);

  }

  public String stop() throws AdminException {

    WebContext.getInstance().getArchiveDaemonManager().stopDaemon();
    return null;
  }

  public int getInterval() {
    return getSettings().getInterval();
  }

  public ArchiveDaemonSettings getConfig() {
    return config;
  }

  public void setConfig(ArchiveDaemonSettings config) {
    this.config = config;
  }

  public DynamicTableModel getLocationSources() {
    return locationSources;
  }

  public void setLocationSources(DynamicTableModel locationSources) {
    this.locationSources = locationSources;
  }

  public DynamicTableModel getIndexatorSmeAddrChunkSizes() {
    return indexatorSmeAddrChunkSizes;
  }

  public void setIndexatorSmeAddrChunkSizes(DynamicTableModel indexatorSmeAddrChunkSizes) {
    this.indexatorSmeAddrChunkSizes = indexatorSmeAddrChunkSizes;
  }

}