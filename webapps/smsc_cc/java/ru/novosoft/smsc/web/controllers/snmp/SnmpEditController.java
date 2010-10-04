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

static final String SEVERITY_REGISTER = "severityRegister";
  static final String SEVERITY_REGISTER_FAILED = "severityRegisterFailed";
  static final String SEVERITY_UNREGISTER = "severityUnregister";
  static final String SEVERITY_UNREGISTER_FAILED="severityUnregisterFailed";

  static final String COUNTER_ACCEPTED="counterAccepted";
  static final String COUNTER_OTHER="counterOther";
  static final String COUNTER_0014="counter0014";
  static final String COUNTER_000d="counter000d";
  static final String COUNTER_FAILED="counterFailed";
  static final String COUNTER_REJECTED="counterRejected";
  static final String COUNTER_DELIVERED="counterDelivered";
  static final String COUNTER_SDP="counterSDP";
  static final String COUNTER_RETIRED="counterRetried";
  static final String COUNTER_0058="counter0058";
  static final String COUNTER_TEMPERROR ="counterTempError";


  private SnmpSettings settings;
  private boolean initError=false;
  private String  objName=null;
  private SnmpObject obj=null;
  private String counterToAdd;
  private String severityToAdd;


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

  public String getCounterToAdd() {
    return counterToAdd;
  }

  public void setCounterToAdd(String counterToAdd) {
    this.counterToAdd = counterToAdd;
  }

  public List<SelectItem> getCountersToAdd() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    if(obj.getCounter000d()==null) ret.add(new SelectItem(COUNTER_000d));
    if(obj.getCounter0014()==null) ret.add(new SelectItem(COUNTER_0014));
    if(obj.getCounter0058()==null) ret.add(new SelectItem(COUNTER_0058));
    if(obj.getCounterAccepted()==null) ret.add(new SelectItem(COUNTER_ACCEPTED));
    if(obj.getCounterDelivered()==null) ret.add(new SelectItem(COUNTER_DELIVERED));
    if(obj.getCounterFailed()==null) ret.add(new SelectItem(COUNTER_FAILED));
    if(obj.getCounterOther()==null) ret.add(new SelectItem(COUNTER_OTHER));
    if(obj.getCounterRejected()==null) ret.add(new SelectItem(COUNTER_REJECTED));
    if(obj.getCounterRetried()==null) ret.add(new SelectItem(COUNTER_RETIRED));
    if(obj.getCounterSDP()==null) ret.add(new SelectItem(COUNTER_SDP));
    if(obj.getCounterTempError()==null) ret.add(new SelectItem(COUNTER_TEMPERROR));
    return ret;
  }

  public String removeCounter() throws AdminException {

    String counterToRemove = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get("counterToRemove");
    if(counterToRemove!=null) {
       if(COUNTER_000d.equals(counterToRemove)) obj.setCounter000d(null);
       else if(COUNTER_0014.equals(counterToRemove)) obj.setCounter0014(null);
       else if(COUNTER_0058.equals(counterToRemove)) obj.setCounter0058(null);
       else if(COUNTER_ACCEPTED.equals(counterToRemove)) obj.setCounterAccepted(null);
       else if(COUNTER_DELIVERED.equals(counterToRemove)) obj.setCounterDelivered(null);
       else if(COUNTER_FAILED.equals(counterToRemove)) obj.setCounterFailed(null);
       else if(COUNTER_OTHER.equals(counterToRemove)) obj.setCounterOther(null);
       else if(COUNTER_REJECTED.equals(counterToRemove)) obj.setCounterRejected(null);
       else if(COUNTER_RETIRED.equals(counterToRemove)) obj.setCounterRetried(null);
       else if(COUNTER_SDP.equals(counterToRemove)) obj.setCounterSDP(null);
       else if(COUNTER_TEMPERROR.equals(counterToRemove)) obj.setCounterTempError(null);
       setSettings(settings);
    }
    return null;
  }


  public String addCounter()  throws AdminException {
    if(counterToAdd==null) return null;

    if(COUNTER_000d.equals(counterToAdd)) {
       if(obj.getCounter000d()==null) {
         obj.setCounter000d(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_0014.equals(counterToAdd)) {
       if(obj.getCounter0014()==null) {
         obj.setCounter0014(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_0058.equals(counterToAdd)) {
       if(obj.getCounter0058()==null) {
         obj.setCounter0058(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_ACCEPTED.equals(counterToAdd)) {
       if(obj.getCounterAccepted()==null) {
         obj.setCounterAccepted(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_DELIVERED.equals(counterToAdd)) {
       if(obj.getCounterDelivered()==null) {
         obj.setCounterDelivered(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_FAILED.equals(counterToAdd)) {
       if(obj.getCounterFailed()==null) {
         obj.setCounterFailed(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_OTHER.equals(counterToAdd)) {
       if(obj.getCounterOther()==null) {
         obj.setCounterOther(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_REJECTED.equals(counterToAdd)) {
       if(obj.getCounterRejected()==null) {
         obj.setCounterRejected(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_RETIRED.equals(counterToAdd)) {
       if(obj.getCounterRetried()==null) {
         obj.setCounterRetried(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_SDP.equals(counterToAdd)) {
       if(obj.getCounterSDP()==null) {
         obj.setCounterSDP(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    if(COUNTER_TEMPERROR.equals(counterToAdd)) {
       if(obj.getCounterTempError()==null) {
         obj.setCounterTempError(new SnmpCounter(0,0,0,0));
         setSettings(settings);
         return null;
       }
    }
    return null;
  }

  //------- severities -----
   public List<SelectItem> getSeveritiesToAdd() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    if(obj.getSeverityRegister()==null) ret.add(new SelectItem(SEVERITY_REGISTER));
    if(obj.getSeverityRegisterFailed()==null) ret.add(new SelectItem(SEVERITY_REGISTER_FAILED));
    if(obj.getSeverityUnregister()==null) ret.add(new SelectItem(SEVERITY_UNREGISTER));
    if(obj.getSeverityUnregisterFailed()==null) ret.add(new SelectItem(SEVERITY_UNREGISTER_FAILED));
    return ret;
  }

  public String getSeverityToAdd() {
    return severityToAdd;
  }

  public void setSeverityToAdd(String severityToAdd) {
    this.severityToAdd = severityToAdd;
  }

  public String removeSeverity() throws AdminException {
    String severityToRemove = getRequestParameter("severityToRemove");
    if(severityToRemove!=null) {
       if(SEVERITY_REGISTER.equals(severityToRemove)) obj.setSeverityRegister(null);
       else if(SEVERITY_REGISTER_FAILED.equals(severityToRemove)) obj.setSeverityRegisterFailed(null);
       else if(SEVERITY_UNREGISTER.equals(severityToRemove)) obj.setSeverityUnregister(null);
       else if(SEVERITY_UNREGISTER_FAILED.equals(severityToRemove)) obj.setSeverityUnregisterFailed(null);
       setSettings(settings);
    }
    return null;
  }

  public String addSeverity()  throws AdminException {
    if(severityToAdd==null) return null;

    if(SEVERITY_REGISTER.equals(severityToAdd)) {
       if(obj.getSeverityRegister()==null) {
         obj.setSeverityRegister(SnmpSeverity.OFF);
         setSettings(settings);
         return null;
       }
    }
    else if(SEVERITY_REGISTER_FAILED.equals(severityToAdd)) {
       if(obj.getSeverityRegisterFailed()==null) {
         obj.setSeverityRegisterFailed(SnmpSeverity.OFF);
         setSettings(settings);
         return null;
       }
    }
    else if(SEVERITY_UNREGISTER.equals(severityToAdd)) {
        if(obj.getSeverityUnregister()==null) {
          obj.setSeverityUnregister(SnmpSeverity.OFF);
          setSettings(settings);
          return null;
        }
    }
    else if(SEVERITY_UNREGISTER_FAILED.equals(severityToAdd)) {
        if(obj.getSeverityUnregisterFailed()==null) {
          obj.setSeverityUnregisterFailed(SnmpSeverity.OFF);
          setSettings(settings);
          return null;
        }
    }
    return null;
  }

  public String getObjTitle() {
    String name = objName;
    if(name==null || name.length()==0) name=getLocalizedString("snmp.default.object");
    return getLocalizedString("snmp.edit.object.subtitle",name);
  }
}
