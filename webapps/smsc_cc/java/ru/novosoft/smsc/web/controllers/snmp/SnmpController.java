package ru.novosoft.smsc.web.controllers.snmp;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.snmp.SnmpObject;
import ru.novosoft.smsc.admin.snmp.SnmpSettings;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.controllers.SettingsController;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.09.2010
 * Time: 13:50:14
 */
public class SnmpController extends SettingsController<SnmpSettings> {
  private static final Logger logger = Logger.getLogger(SnmpController.class);

  SnmpSettings snmpSettings;
  private String counterInterval;
  private boolean defaultOpened;
  private String defaultEnabled;
  private boolean defaultCountersOpened;
  private boolean defaultSeveritiesOpened;
  private String newObject;
  private Object defaultObject;


  public SnmpController() {
    super(ConfigType.Snmp);
    snmpSettings= getSettings();
    checkOutOfDate();
  }

  @Override
  protected SnmpSettings loadSettings() throws AdminException {
    return getConfiguration().getSnmpSettings();
  }

  @Override
  protected void saveSettings(SnmpSettings settings) throws AdminException {
    getConfiguration().updateSnmpSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected SnmpSettings cloneSettings(SnmpSettings settings) {
    return settings.cloneSettings();
  }

  private void checkOutOfDate() {
    try {
      List<Integer> result = new ArrayList<Integer>();
      SmscStatusManager ssm = getSmscStatusManager();
      for (int i = 0; i < ssm.getSmscInstancesNumber(); i++) {
        if (ssm.getSnmpConfigState(i) == SmscConfigurationStatus.OUT_OF_DATE)
          result.add(i);
      }
      if (!result.isEmpty())
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.config.instance.out_of_date", result.toString());
    } catch (AdminException e) {
      logger.error(e, e);
      addError(e);
    }
  }

  public String reset() {
    try {
      resetSettings();
    } catch (AdminException e) {
      addError(e);
    }
    return "SNMP";
  }

  public String save() {
    try {
      //frSettings.setWhiteList(wl);
      setSettings(snmpSettings);
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

  public void setCounterInterval(int counterInterval) throws AdminException {
    snmpSettings.setCounterInterval(counterInterval);
  }

  public int getCounterInterval() {
    return snmpSettings.getCounterInterval();
  }


  public boolean getDefaultEnabled() {
    return snmpSettings.getDefaultSnmpObject().isEnabled();
  }
  
  public void setDefaultEnabled(boolean v) {
    snmpSettings.getDefaultSnmpObject().setEnabled(v);
  }



  public void setNewObject(String newObject) {
    this.newObject = newObject;
  }

  public String getNewObject() {
    return newObject;
  }

  public String addObject() throws AdminException {
    if(newObject!=null && newObject.trim().length()>0) {
      Map<String, SnmpObject> snmpObjects = snmpSettings.getSnmpObjects();

      if(!snmpObjects.containsKey(newObject)) {
        snmpObjects.put(newObject,new SnmpObject());
        snmpSettings.setSnmpObjects(snmpObjects);
        setSettings(snmpSettings);
      }
      else {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "snmp.object.exists");
      }
    }
    return null;
  }


  public List<Map.Entry<String, SnmpObject>> getSnmpObjects() {
    ArrayList<Map.Entry<String, SnmpObject>> ret = new ArrayList<Map.Entry<String, SnmpObject>>(snmpSettings.getSnmpObjects().entrySet());
    Collections.sort(ret,new Comparator<Map.Entry<String, SnmpObject>>(){
      public int compare(String s, String s1) {
        return s.compareTo(s1 );
      }
      public int compare(Map.Entry<String, SnmpObject> o, Map.Entry<String, SnmpObject> o1) {
              return o.getKey().compareTo(o1.getKey() );
      }
    });
    return ret;
  }

  public String removeObject() throws AdminException {
    String objToRemove = getRequestParameter("objToRemove");
    snmpSettings.getSnmpObjects().remove(objToRemove);
    setSettings(snmpSettings);
    return null;
  }


  public void setDefaultObject(SnmpObject defaultObject) {
    this.defaultObject = defaultObject;
  }

  public SnmpObject getDefaultObject() {
    return snmpSettings.getDefaultSnmpObject();
  }
}
