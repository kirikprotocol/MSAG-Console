package ru.novosoft.smsc.web.controllers.map_limit;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.config.AppliableConfiguration;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import java.text.MessageFormat;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * author: alkhal
 */
public class MapLimitController extends SmscController{

  private static final Logger logger = Logger.getLogger(MapLimitController.class);

  private AppliableConfiguration conf;

  private boolean initialized = false;

  private DynamicTableModel codes = new DynamicTableModel();

  private long lastUpdate;

  private MapLimitSettings mlSettings;

  public MapLimitController() {

    Map<String, String> reguestMap = getRequestParameters();
    conf = WebContext.getInstance().getAppliableConfiguration();

    try {
      if (!reguestMap.containsKey("initialized")) {
        lastUpdate = conf.getMapLimitSettingsUpdateInfo().getLastUpdateTime();
        init();
      }
      List<Integer> outOfDate = new LinkedList<Integer>();

      SmscStatusManager smscStatusManager = WebContext.getInstance().getSmscStatusManager();
      for (int i = 0; i < smscStatusManager.getSmscInstancesNumber(); i++) {
        if (smscStatusManager.getMapLimitState(i) == SmscConfigurationStatus.OUT_OF_DATE)
          outOfDate.add(i);
      }

      if (!outOfDate.isEmpty()) {
        String message = MessageFormat.format(
            ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", getLocale()).getString("smsc.config.instance.out_of_date"),
            outOfDate.toString());
        addMessage(FacesMessage.SEVERITY_WARN, message);
      }
    } catch (AdminException e) {
      logger.error(e, e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
    }

  }

  private void init() {
    try{
      mlSettings = conf.getMapLimitSettings();
      for(int c : mlSettings.getUssdNoSriCodes()) {
        DynamicTableRow row = new DynamicTableRow();
        row.setValue("code", c);
        codes.addRow(row);
      }
    }catch (AdminException e){
      logger.error(e,e);
    }
  }

  public String save() {
    if (lastUpdate != conf.getMapLimitSettingsUpdateInfo().getLastUpdateTime()) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual");
      return null;
    }

    int noSriCodes[] = new int[codes.getRowCount()];
    int i=0;
    for (DynamicTableRow row : codes.getRows()) {
      String value = (String) row.getValue("code");
      if (value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.maplimits.ussdnosricodes.empty");
        return null;
      }
      try{
        int n =  Integer.parseInt(value);
        if(n<=0) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.maplimits.ussdnosricodes.convert");
          return null;
        }
        noSriCodes[i] = n;
        i++;
      }catch (Exception e){
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.maplimits.ussdnosricodes.convert");
        return null;
      }
    }

    try{
      mlSettings.setUssdNoSriCodes(noSriCodes);
      conf.setMapLimitSettings(mlSettings, getUserPrincipal().getName());
    }catch (AdminException e){
      logger.error(e,e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
    }

    return "INDEX";
  }

  public DynamicTableModel getCodes() {
    return codes;
  }

  public void setCodes(DynamicTableModel codes) {
    this.codes = codes;
  }

  public long getLastUpdate() {
    return lastUpdate;
  }

  public void setLastUpdate(long lastUpdate) {
    this.lastUpdate = lastUpdate;
  }

  public MapLimitSettings getMlSettings() {
    return mlSettings;
  }

  public void setMlSettings(MapLimitSettings mlSettings) {
    this.mlSettings = mlSettings;
  }

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }
}
