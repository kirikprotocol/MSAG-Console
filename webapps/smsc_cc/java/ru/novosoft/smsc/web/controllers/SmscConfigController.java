package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.config.AppliableConfiguration;

import javax.faces.application.FacesMessage;
import java.security.Principal;
import java.text.MessageFormat;
import java.util.*;

/**
 * author: alkhal
 */
public class SmscConfigController extends SmscController{

  private static final Logger logger = Logger.getLogger(SmscConfigController.class);

  private AppliableConfiguration conf;

  private int instancesCount = 0;

  private CommonSettings commonConfig= new CommonSettings();

  private List<InstanceSettings> instanceConfigs = new LinkedList<InstanceSettings>();

  private boolean initialized = false;

  private DynamicTableModel addSsnModel = new DynamicTableModel();

  private DynamicTableModel localesModel = new DynamicTableModel();

  private DynamicTableModel directivesModel = new DynamicTableModel();

  private long lastUpdate;


  public SmscConfigController() {
    Map<String, String> reguestMap = getRequestParameters();
    conf = WebContext.getInstance().getAppliableConfiguration();
    SmscSettings smscSettings = conf.getSmscSettings();
    if(!reguestMap.containsKey("initialized")) {
      lastUpdate = conf.getSmscSettigsUpdateInfo().getLastUpdateTime();
      instancesCount = smscSettings.getSmscInstancesCount();        
      initCommonConfig();
      initInstances();
    }
    try{
      List<Integer> outOfDate = new LinkedList<Integer>();
      for(Map.Entry<Integer, SmscConfigurationStatus> e : conf.getSmscSettingsStatus().entrySet()) {
        if(e.getValue() == SmscConfigurationStatus.OUT_OF_DATE) {
          outOfDate.add(e.getKey());
        }
      }
      if(!outOfDate.isEmpty()) {
        String message = MessageFormat.format(
            ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc", getLocale()).getString("smsc.config.instance.out_of_date"),
            outOfDate.toString());
        addMessage(FacesMessage.SEVERITY_WARN, message);
      }
    }catch (AdminException e) {
      logger.error(e,e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
    }
  }

  private void initCommonConfig() {
    commonConfig = conf.getSmscSettings().getCommonSettings();

    //core
    for(String ssn : commonConfig.getAdd_ussd_ssn()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("ssn", ssn);
      addSsnModel.addRow(row);
    }
    for(String l : commonConfig.getLocales()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("locale", l);
      localesModel.addRow(row);
    }

    // directives

    for(Map.Entry<String, String> e : commonConfig.getDirectives().entrySet()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("key", e.getKey());
      row.setValue("value", e.getValue());
      directivesModel.addRow(row);
    }

  }

  private void initInstances() {
    for (int i = 0; i < instancesCount; i++) {
      instanceConfigs.add(conf.getSmscSettings().getInstanceSettings(i));
    }
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

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public void setInstancesCount(int instancesCount) {
    this.instancesCount = instancesCount;
  }

  public long getLastUpdate() {
    return lastUpdate;
  }

  public void setLastUpdate(long lastUpdate) {
    this.lastUpdate = lastUpdate;
  }

  public String save() {

    if(lastUpdate != conf.getSmscSettigsUpdateInfo().getLastUpdateTime()) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual");
      return null;
    }

    for(DynamicTableRow row : addSsnModel.getRows()) {
      String value = (String)row.getValue("ssn");
      if(value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "add_ssn.empty");
        return null;
      }
    }
    for(DynamicTableRow row : localesModel.getRows()) {
      String value = (String)row.getValue("locale");
      if(value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "locale.empty");
        return null;
      }
    }
    for(DynamicTableRow row : directivesModel.getRows()) {
      String value = (String)row.getValue("key");
      if(value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "directives.empty");   
        return null;        
      }
    }

    Locale locale = getLocale();
    try{
      CommonSettings cs = fill(commonConfig);
      InstanceSettings[] iss = new InstanceSettings[instancesCount];
      for(int i=0;i<instancesCount;i++) {
        iss[i] = instanceConfigs.get(i);
      }

      SmscSettings smscSettings = conf.getSmscSettings();

      smscSettings.setCommonSettings(cs);
      for(int i=0;i<instancesCount;i++) {
        smscSettings.setInstanceSettings(i, iss[i]);
      }

      Principal p = getUserPrincipal();

      conf.setSmscSettings(smscSettings, p.getName());

      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e,e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(locale));
      return null;
    }

  }


  private CommonSettings fill(CommonSettings commonSettings) throws AdminException {
    String[] aus = new String[addSsnModel.getRowCount()];

    int i=0;
    for(DynamicTableRow s : addSsnModel.getRows()) {
      aus[i] = (String)s.getValue("ssn");
      i++;
    }
    commonSettings.setAdd_ussd_ssn(aus);

    String[] ls = new String[localesModel.getRowCount()];
    i=0;
    for(DynamicTableRow s : localesModel.getRows()) {
      ls[i] = (String)s.getValue("locale");
      i++;
    }
    commonSettings.setLocales(ls);

    // directives

    Map<String,String> d = new HashMap<String, String>(directivesModel.getRowCount());
    for(DynamicTableRow e : directivesModel.getRows()) {
      d.put((String)e.getValue("key"), (String)e.getValue("value"));
    }
    commonSettings.setDirectives(d);

    return commonSettings;
  }
}
