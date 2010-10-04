package ru.novosoft.smsc.web.controllers.snmp;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.snmp.SnmpObject;
import ru.novosoft.smsc.admin.snmp.SnmpSettings;
import ru.novosoft.smsc.admin.snmp.SnmpSeverity;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.09.2010
 * Time: 13:50:14
 */
public class SnmpController extends SettingsMController<SnmpSettings> {
  private static final Logger logger = Logger.getLogger(SnmpController.class);

  SnmpSettings snmpSettings;
  private String newObjectName;
  private Object snmpObjectNames;
  boolean initError=false;


  public SnmpController() {
    super(WebContext.getInstance().getSnmpManager());
    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initError = true;
      
    }

  }

  @Override
  protected void init() throws AdminException {
    super.init();    //To change body of overridden methods use File | Settings | File Templates.
    snmpSettings=getSettings();
  }


  private void checkChanges() {
    if (isSettingsChanged())
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "smsc.submit.hint");
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
      submitSettings();
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



  public void setNewObjectName(String newObject) {
    this.newObjectName = newObject;
  }

  public String getNewObjectName() {
    return newObjectName;
  }

  public String addObject() throws AdminException {
    if(newObjectName!=null && newObjectName.trim().length()>0) {
      Map<String, SnmpObject> snmpObjects = snmpSettings.getSnmpObjects();
      if(!snmpObjects.containsKey(newObjectName)) {
        SnmpObject o = new SnmpObject();
        snmpObjects.put(newObjectName,new SnmpObject());
        setSettings(snmpSettings);
      }
      else {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "snmp.object.exists");
      }
    }
    checkChanges();
    return null;
  }


  public String removeObject() throws AdminException {
    String objToRemove = getRequestParameter("objToRemove");
    snmpSettings.getSnmpObjects().remove(objToRemove);
    setSettings(snmpSettings);
    checkChanges();
    return null;
  }



  public SnmpObject getDefaultObject() {
    return snmpSettings.getDefaultSnmpObject();
  }

  public List<SelectItem> getSeverityOptions() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    for(SnmpSeverity s : SnmpSeverity.values()) {
      ret.add(new SelectItem(s));
    }
    return ret;
  }
  
  public Object editObject() {
    return "SNMP_EDIT";
  }

  public List<String> getSnmpObjectNames() {
    return new ArrayList<String>(snmpSettings.getSnmpObjects().keySet());
  }

  public void setSettings() {
    setSettings(snmpSettings);
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }
}
