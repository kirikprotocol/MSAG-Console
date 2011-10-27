package ru.novosoft.smsc.web.controllers.archive_daemon;

import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.application.FacesMessage;
import java.util.*;


public class ArchiveDaemonController extends SettingsMController<ArchiveDaemonSettings> {

  private boolean initError;
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

  public boolean isArchiveDaemonDeployed() {
    return WebContext.getInstance().getArchiveDaemonManager() != null;
  }

  @Override
  protected void init() throws AdminException {
    super.init();
    config=getSettings();
    locationSources = buildModel(config.getLocationsSources().entrySet());
    indexatorSmeAddrChunkSizes = buildModel(config.getIndexatorSmeAddrChunkSizes().entrySet());
  }

  public ArchiveDaemonController() {
    super(WebContext.getInstance().getArchiveDaemonManager());
    if (WebContext.getInstance().getArchiveDaemonManager() == null) {
      initError = true;
      return;
    }
    
    try {
      init();

    } catch (AdminException e) {
      addError(e);
      initError = true;
      return;
    }
  }

  public boolean isInitError() {
    return initError;
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