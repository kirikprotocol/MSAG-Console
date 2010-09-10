package ru.novosoft.smsc.web.controllers.smsc_config;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.controllers.SettingsController;

import javax.faces.application.FacesMessage;
import java.util.*;

/**
 * author: alkhal
 */
public class SmscConfigController extends SettingsController<SmscSettings> {

  private static final Logger logger = Logger.getLogger(SmscConfigController.class);

  private int instancesCount = 0;

  private CommonSettings commonConfig = new CommonSettings();

  private List<InstanceSettings> instanceConfigs;

  private DynamicTableModel addSsnModel = new DynamicTableModel();

  private DynamicTableModel localesModel = new DynamicTableModel();

  private DynamicTableModel directivesModel = new DynamicTableModel();

  public SmscConfigController() {
    super(ConfigType.Main);

    checkOutOfDate();

    if (isSettingsChanged())
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.configuration.locally.changed");    

    init();
  }

  private void init() {
    SmscSettings s = getSettings();

    commonConfig = s.getCommonSettings();

    addSsnModel = new DynamicTableModel();
    for (String ssn : commonConfig.getAdd_ussd_ssn()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("ssn", ssn);
      addSsnModel.addRow(row);
    }

    localesModel = new DynamicTableModel();
    for (String l : commonConfig.getLocales()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("locale", l);
      localesModel.addRow(row);
    }

    directivesModel = new DynamicTableModel();
    for (Map.Entry<String, String> e : commonConfig.getDirectives().entrySet()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("key", e.getKey());
      row.setValue("value", e.getValue());
      directivesModel.addRow(row);
    }

    instancesCount = s.getSmscInstancesCount();
    instanceConfigs = new ArrayList<InstanceSettings>(instancesCount);
    for (int i = 0; i < instancesCount; i++)
      instanceConfigs.add(s.getInstanceSettings(i));
  }

  public DynamicTableModel getAddSsnModel() {
    return addSsnModel;
  }

  public void setAddSsnModel(DynamicTableModel addSsnModel) {
    this.addSsnModel = addSsnModel;
  }

  public DynamicTableModel getLocalesModel() {
    return localesModel;
  }

  public void setLocalesModel(DynamicTableModel localesModel) {
    this.localesModel = localesModel;
  }

  public DynamicTableModel getDirectivesModel() {
    return directivesModel;
  }

  public void setDirectivesModel(DynamicTableModel directivesModel) {
    this.directivesModel = directivesModel;
  }

  public int getInstancesCount() {
    return instancesCount;
  }


  public List<InstanceSettings> getInstanceConfigs() {
    return instanceConfigs;
  }

  public CommonSettings getCommonConfig() {
    return commonConfig;
  }


  public void setCommonConfig(CommonSettings commonConfig) {
    this.commonConfig = commonConfig;
  }

  public void setInstanceConfigs(List<InstanceSettings> instanceConfigs) {
    this.instanceConfigs = instanceConfigs;
  }

  public void setInstancesCount(int instancesCount) {
    this.instancesCount = instancesCount;
  }

  public String reset() {
    try {
      resetSettings();
      init();
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String save() {

    for (DynamicTableRow row : addSsnModel.getRows()) {
      String value = (String) row.getValue("ssn");
      if (value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "add_ssn.empty");
        return null;
      }
    }
    for (DynamicTableRow row : localesModel.getRows()) {
      String value = (String) row.getValue("locale");
      if (value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "locale.empty");
        return null;
      }
    }
    for (DynamicTableRow row : directivesModel.getRows()) {
      String value = (String) row.getValue("key");
      if (value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "directives.empty");
        return null;
      }
    }

    try {
      CommonSettings cs = fill(commonConfig);
      InstanceSettings[] iss = new InstanceSettings[instancesCount];
      for (int i = 0; i < instancesCount; i++) {
        iss[i] = instanceConfigs.get(i);
      }

      SmscSettings smscSettings = getSettings();

      smscSettings.setCommonSettings(cs);
      for (int i = 0; i < instancesCount; i++) {
        smscSettings.setInstanceSettings(i, iss[i]);
      }

      setSettings(smscSettings);
      Revision rev = submitSettings();
      if (rev != null) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual", rev.getUser());
        return null;
      }

      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e, e);
      addError(e);
      return null;
    }
  }


  private CommonSettings fill(CommonSettings commonSettings) throws AdminException {
    String[] aus = new String[addSsnModel.getRowCount()];

    int i = 0;
    for (DynamicTableRow s : addSsnModel.getRows()) {
      aus[i] = (String) s.getValue("ssn");
      i++;
    }
    commonSettings.setAdd_ussd_ssn(aus);

    String[] ls = new String[localesModel.getRowCount()];
    i = 0;
    for (DynamicTableRow s : localesModel.getRows()) {
      ls[i] = (String) s.getValue("locale");
      i++;
    }
    commonSettings.setLocales(ls);

    // directives

    Map<String, String> d = new HashMap<String, String>(directivesModel.getRowCount());
    for (DynamicTableRow e : directivesModel.getRows()) {
      d.put((String) e.getValue("key"), (String) e.getValue("value"));
    }
    commonSettings.setDirectives(d);

    return commonSettings;
  }

  private void checkOutOfDate() {
    try {
      List<Integer> result = new ArrayList<Integer>();
      SmscStatusManager ssm = getSmscStatusManager();
      for (int i = 0; i < ssm.getSmscInstancesNumber(); i++) {
        if (ssm.getMainConfigState(i) == SmscConfigurationStatus.OUT_OF_DATE)
          result.add(i);
      }
      if (!result.isEmpty())
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.config.instance.out_of_date", result.toString());
    } catch (AdminException e) {
      logger.error(e, e);
      addError(e);
    }
  }

  @Override
  protected SmscSettings loadSettings() throws AdminException {
    return getConfiguration().getSmscSettings();
  }

  @Override
  protected void saveSettings(SmscSettings settings) throws AdminException {
    getConfiguration().updateSmscSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected SmscSettings cloneSettings(SmscSettings settings) {
    return settings.cloneSettings();
  }
}
