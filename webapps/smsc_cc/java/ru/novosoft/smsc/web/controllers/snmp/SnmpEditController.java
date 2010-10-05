package ru.novosoft.smsc.web.controllers.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.snmp.SnmpCounter;
import ru.novosoft.smsc.admin.snmp.SnmpObject;
import ru.novosoft.smsc.admin.snmp.SnmpSettings;
import ru.novosoft.smsc.admin.snmp.SnmpSeverity;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.context.FacesContext;
import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 04.10.2010
 * Time: 14:42:53
 */
public class SnmpEditController  extends SettingsMController<SnmpSettings> {

  public enum SeverityName {
    severityRegister,severityRegisterFailed,severityUnregister,severityUnregisterFailed
  }

  public enum CounterName {
    counterAccepted,counterOther,counter0014,counter000d,counterFailed,
    counterRejected,counterDelivered,counterSDP,counterRetried,counter0058,
    counterTempError
  }




  private SnmpSettings settings;
  private boolean initError=false;
  private String  objName=null;
  private SnmpObject obj=null;
  private CounterName counterToAdd;
  private SeverityName severityToAdd;


  @Override
  protected void init() throws AdminException {
    super.init(false);    //To change body of overridden methods use File | Settings | File Templates.
    settings = getSettings();
    initObject();
  }

  private void initObject() {
    String objName = getRequestParameter("objName");
    if(objName!=null) {
      setObjName(objName);
    }
  }

  public SnmpEditController() {
    super(WebContext.getInstance().getSnmpManager());
    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initError = true;
    }
  }

  public String getObjName() {
    return objName;
  }

  public void setObjName(String objName) {
    this.objName = objName;
    if(objName==null || objName.length()==0) obj = settings.getDefaultSnmpObject();
    else {
      obj = settings.getSnmpObjects().get(objName);
    }
  }

  public SnmpObject getObj() {
    return obj;
  }

  public void setObj(SnmpObject obj) {
    this.obj = obj;
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }


  public String done() {
    setSettings(settings);
    return "SNMP";
  }

  public String cancel() {
    return "SNMP";
  }

  //-------- counters -------

  public CounterName getCounterToAdd() {
    return counterToAdd;
  }

  public void setCounterToAdd(CounterName counterToAdd) {
    this.counterToAdd = counterToAdd;
  }

  public List<SelectItem> getCountersToAdd() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    if(obj.getCounter000d()==null) ret.add(new SelectItem(CounterName.counter000d));
    if(obj.getCounter0014()==null) ret.add(new SelectItem(CounterName.counter0014));
    if(obj.getCounter0058()==null) ret.add(new SelectItem(CounterName.counter0058));
    if(obj.getCounterAccepted()==null) ret.add(new SelectItem(CounterName.counterAccepted));
    if(obj.getCounterDelivered()==null) ret.add(new SelectItem(CounterName.counterDelivered));
    if(obj.getCounterFailed()==null) ret.add(new SelectItem(CounterName.counterFailed));
    if(obj.getCounterOther()==null) ret.add(new SelectItem(CounterName.counterOther));
    if(obj.getCounterRejected()==null) ret.add(new SelectItem(CounterName.counterRejected));
    if(obj.getCounterRetried()==null) ret.add(new SelectItem(CounterName.counterRetried));
    if(obj.getCounterSDP()==null) ret.add(new SelectItem(CounterName.counterSDP));
    if(obj.getCounterTempError()==null) ret.add(new SelectItem(CounterName.counterTempError));
    return ret;
  }

  public String removeCounter() throws AdminException {
    String s = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get("counterToRemove");
    if(s==null) return null;
    CounterName counterToRemove = CounterName.valueOf(s);
    switch (counterToRemove) {
      case counter000d:
        obj.setCounter000d(null);
        break;
      case counter0014:
        obj.setCounter0014(null);
        break;
      case counter0058:
        obj.setCounter0058(null);
        break;
      case counterAccepted:
        obj.setCounterAccepted(null);
        break;
      case counterDelivered:
        obj.setCounterDelivered(null);
        break;
      case counterFailed:
        obj.setCounterFailed(null);
        break;
      case counterOther:
        obj.setCounterOther(null);
        break;
      case counterRejected:
        obj.setCounterRejected(null);
        break;
      case counterRetried:
        obj.setCounterRetried(null);
        break;
      case counterSDP:
        obj.setCounterSDP(null);
        break;
      case counterTempError:
        obj.setCounterTempError(null);
        break;
    }
    setSettings(settings);
    return null;
  }


  public String addCounter()  throws AdminException {
    if(counterToAdd==null) return null;
    switch (counterToAdd) {

      case counter000d :
        if(obj.getCounter000d()==null)
          obj.setCounter000d(new SnmpCounter(0,0,0,0));
        break;

      case counter0014:
        if(obj.getCounter0014()==null)
          obj.setCounter0014(new SnmpCounter(0,0,0,0));
        break;
      case counter0058:
        if(obj.getCounter0058()==null)
          obj.setCounter0058(new SnmpCounter(0,0,0,0));
        break;

      case counterAccepted:
        if(obj.getCounterAccepted()==null)
          obj.setCounterAccepted(new SnmpCounter(0,0,0,0));
        break;
      case counterDelivered:
        if(obj.getCounterDelivered()==null)
          obj.setCounterDelivered(new SnmpCounter(0,0,0,0));
        break;
      case counterFailed:
        if(obj.getCounterFailed()==null)
          obj.setCounterFailed(new SnmpCounter(0,0,0,0));
        break;
      case counterOther:
        if(obj.getCounterOther()==null)
          obj.setCounterOther(new SnmpCounter(0,0,0,0));
        break;
      case counterRejected:
        if(obj.getCounterRejected()==null)
          obj.setCounterRejected(new SnmpCounter(0,0,0,0));
        break;
      case counterRetried:
        if(obj.getCounterRetried()==null)
          obj.setCounterRetried(new SnmpCounter(0,0,0,0));
        break;
      case counterSDP:
        if(obj.getCounterSDP()==null)
          obj.setCounterSDP(new SnmpCounter(0,0,0,0));
        break;
      case counterTempError:
        if(obj.getCounterTempError()==null)
          obj.setCounterTempError(new SnmpCounter(0,0,0,0));
        break;
    }
    setSettings(settings);
    return null;
  }

  //------- severities -----
  public List<SelectItem> getSeveritiesToAdd() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    if(obj.getSeverityRegister()==null)
      ret.add(new SelectItem(SeverityName.severityRegister));
    if(obj.getSeverityRegisterFailed()==null)
      ret.add(new SelectItem(SeverityName.severityRegisterFailed));
    if(obj.getSeverityUnregister()==null)
      ret.add(new SelectItem(SeverityName.severityUnregister));
    if(obj.getSeverityUnregisterFailed()==null)
      ret.add(new SelectItem(SeverityName.severityUnregisterFailed));
    return ret;
  }

  public SeverityName getSeverityToAdd() {
    return severityToAdd;
  }

  public void setSeverityToAdd(SeverityName severityToAdd) {
    this.severityToAdd = severityToAdd;
  }

  public String removeSeverity() throws AdminException {
    SeverityName severityToRemove = SeverityName.valueOf(getRequestParameter("severityToRemove"));
    if(severityToRemove==null) return null;

    switch (severityToRemove) {
      case severityRegister :
        obj.setSeverityRegister(null);
        break;
      case severityRegisterFailed:
        obj.setSeverityRegisterFailed(null);
        break;
      case severityUnregister:
        obj.setSeverityUnregister(null);
        break;
      case severityUnregisterFailed:
        obj.setSeverityUnregisterFailed(null);
        break;
    }
    setSettings(settings);
    return null;
  }

  public String addSeverity()  throws AdminException {
    if(severityToAdd==null) return null;
    switch (severityToAdd) {
      case severityRegister :
        if(obj.getSeverityRegister()==null) {
          obj.setSeverityRegister(SnmpSeverity.OFF);
          setSettings(settings);
        }
        break;

      case severityRegisterFailed :
        if(obj.getSeverityRegisterFailed()==null) {
          obj.setSeverityRegisterFailed(SnmpSeverity.OFF);
          setSettings(settings);
        }
        break;

      case severityUnregister :
        if(obj.getSeverityUnregister()==null) {
          obj.setSeverityUnregister(SnmpSeverity.OFF);
          setSettings(settings);
        }
        break;

      case severityUnregisterFailed:
        if(obj.getSeverityUnregisterFailed()==null) {
          obj.setSeverityUnregisterFailed(SnmpSeverity.OFF);
          setSettings(settings);
        }
        break;
    }
    return null;
  }

  public String getObjTitle() {
    String name = (objName==null || objName.length()==0) ? getLocalizedString("snmp.default.object") : objName;
    return getLocalizedString("snmp.edit.object.subtitle",name);
  }
}
