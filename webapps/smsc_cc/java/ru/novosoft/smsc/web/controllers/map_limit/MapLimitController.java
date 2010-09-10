package ru.novosoft.smsc.web.controllers.map_limit;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.controllers.SettingsController;

import javax.faces.application.FacesMessage;
import java.util.LinkedList;
import java.util.List;

/**
 * author: alkhal
 */
public class MapLimitController extends SettingsController<MapLimitSettings> {

  private static final Logger logger = Logger.getLogger(MapLimitController.class);

  private DynamicTableModel codes = new DynamicTableModel();
  private MapLimitSettings mlSettings;

  public MapLimitController() {

    super(ConfigType.MapLimit);

    if (isSettingsChanged())
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.configuration.locally.changed");

    try {

      mlSettings = getSettings();

      List<Integer> outOfDate = new LinkedList<Integer>();

      SmscStatusManager smscStatusManager = WebContext.getInstance().getSmscStatusManager();
      for (int i = 0; i < smscStatusManager.getSmscInstancesNumber(); i++) {
        if (smscStatusManager.getMapLimitState(i) == SmscConfigurationStatus.OUT_OF_DATE)
          outOfDate.add(i);
      }
      if (!outOfDate.isEmpty())
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.config.instance.out_of_date", outOfDate.toString());

      for (int c : mlSettings.getUssdNoSriCodes()) {
        DynamicTableRow row = new DynamicTableRow();
        row.setValue("code", c);
        codes.addRow(row);
      }
    } catch (AdminException e) {
      logger.error(e, e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
    }

  }

  public String reset() {
    try {
      resetSettings();
    } catch (AdminException e) {
      addError(e);
    }
    return "MAP_LIMIT";
  }

  public String save() {

    int noSriCodes[] = new int[codes.getRowCount()];
    int i = 0;
    for (DynamicTableRow row : codes.getRows()) {
      String value = (String) row.getValue("code");
      if (value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.maplimits.ussdnosricodes.empty");
        return null;
      }
      try {
        int n = Integer.parseInt(value);
        if (n <= 0) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.maplimits.ussdnosricodes.convert");
          return null;
        }
        noSriCodes[i] = n;
        i++;
      } catch (Exception e) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.maplimits.ussdnosricodes.convert");
        return null;
      }
    }

    try {
      mlSettings.setUssdNoSriCodes(noSriCodes);
      setSettings(mlSettings);

      Revision rev = submitSettings();
      if (rev != null) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual", rev.getUser());
        return null;
      }

    } catch (AdminException e) {
      logger.error(e, e);
      addError(e);
    }

    return "INDEX";
  }

  public DynamicTableModel getCodes() {
    return codes;
  }

  public void setCodes(DynamicTableModel codes) {
    this.codes = codes;
  }

  public MapLimitSettings getMlSettings() {
    return mlSettings;
  }

  public void setMlSettings(MapLimitSettings mlSettings) {
    this.mlSettings = mlSettings;
  }

  @Override
  protected MapLimitSettings loadSettings() throws AdminException {
    return getConfiguration().getMapLimitSettings();
  }

  @Override
  protected void saveSettings(MapLimitSettings settings) throws AdminException {
    getConfiguration().updateMapLimitSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected MapLimitSettings cloneSettings(MapLimitSettings settings) {
    return settings.cloneSettings();
  }
}
