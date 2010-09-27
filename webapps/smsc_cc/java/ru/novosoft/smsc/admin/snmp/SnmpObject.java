package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;

/**
 * Структура, описывающая Snmp объект
 * @author Artem Snopkov
 */
public class SnmpObject {

  private boolean enabled;

  private SnmpCounter counterOther;
  private SnmpCounter counter0014;
  private SnmpCounter counter000d;
  private SnmpCounter counterFailed;
  private SnmpCounter counterAccepted;
  private SnmpCounter counterRejected;
  private SnmpCounter counterDelivered;
  private SnmpCounter counterSDP;
  private SnmpCounter counterRetried;
  private SnmpCounter counter0058;
  private SnmpCounter counterTempError;

  private SnmpSeverity severityRegister;
  private SnmpSeverity severityRegisterFailed;
  private SnmpSeverity severityUnregister;
  private SnmpSeverity severityUnregisterFailed;

  public SnmpObject() {
  }

  /**
   * Конструктор копирования
   *
   * @param o копия
   */
  public SnmpObject(SnmpObject o) {
    this.enabled = o.enabled;
    this.counterOther = o.counterOther;
    this.counter0014 = o.counter0014;
    this.counter000d = o.counter000d;
    this.counterFailed = o.counterFailed;
    this.counterAccepted = o.counterAccepted;
    this.counterRejected = o.counterRejected;
    this.counterDelivered = o.counterDelivered;
    this.counterSDP = o.counterSDP;
    this.counterRetried = o.counterRetried;
    this.counter0058 = o.counter0058;
    this.counterTempError = o.counterTempError;

    this.severityRegister = o.severityRegister;
    this.severityRegisterFailed = o.severityRegisterFailed;
    this.severityUnregister = o.severityUnregister;
    this.severityUnregisterFailed = o.severityUnregisterFailed;
  }

  public boolean isEnabled() {
    return enabled;
  }

  public void setEnabled(boolean enabled) {
    this.enabled = enabled;
  }

  public SnmpCounter getCounterOther() {
    return counterOther;
  }

  public void setCounterOther(SnmpCounter counterOther) throws AdminException {
    this.counterOther = counterOther;
  }

  public void setCounterOther(int warning, int minor, int major, int critical) throws AdminException {
    setCounterOther(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounter0014() {
    return counter0014;
  }

  public void setCounter0014(SnmpCounter counter0014) throws AdminException {
    this.counter0014 = counter0014;

  }

  public void setCounter0014(int warning, int minor, int major, int critical) throws AdminException {
     setCounter0014(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounter000d() {
    return counter000d;
  }

  public void setCounter000d(SnmpCounter counter000d) throws AdminException {
    this.counter000d = counter000d;

  }

  public void setCounter000d(int warning, int minor, int major, int critical) throws AdminException {
    setCounter000d(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterFailed() {
    return counterFailed;
  }

  public void setCounterFailed(SnmpCounter counterFailed) throws AdminException {
    this.counterFailed = counterFailed;
  }

  public void setCounterFailed(int warning, int minor, int major, int critical) throws AdminException {
    setCounterFailed(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterAccepted() {
    return counterAccepted;
  }

  public void setCounterAccepted(SnmpCounter counterAccepted) throws AdminException {
    this.counterAccepted = counterAccepted;
  }

  public void setCounterAccepted(int warning, int minor, int major, int critical) throws AdminException {
    setCounterAccepted(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterRejected() {
    return counterRejected;
  }

  public void setCounterRejected(SnmpCounter counterRejected) throws AdminException {
    this.counterRejected = counterRejected;
  }

  public void setCounterRejected(int warning, int minor, int major, int critical) throws AdminException {
    setCounterRejected(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterDelivered() {
    return counterDelivered;
  }

  public void setCounterDelivered(SnmpCounter counterDelivered) throws AdminException {
    this.counterDelivered = counterDelivered;

  }

  public void setCounterDelivered(int warning, int minor, int major, int critical) throws AdminException {
    setCounterDelivered(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterSDP() {
    return counterSDP;
  }

  public void setCounterSDP(SnmpCounter counterSDP) throws AdminException {
    this.counterSDP = counterSDP;
  }

  public void setCounterSDP(int warning, int minor, int major, int critical) throws AdminException {
    setCounterSDP(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterRetried() {
    return counterRetried;
  }

  public void setCounterRetried(SnmpCounter counterRetried) throws AdminException {
    this.counterRetried = counterRetried;
  }

  public void setCounterRetried(int warning, int minor, int major, int critical) throws AdminException {
    setCounterRetried(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounter0058() {
    return counter0058;
  }

  public void setCounter0058(SnmpCounter counter0058) throws AdminException {
    this.counter0058 = counter0058;
  }

  public void setCounter0058(int warning, int minor, int major, int critical) throws AdminException {
    setCounter0058(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterTempError() {
    return counterTempError;
  }

  public void setCounterTempError(SnmpCounter counterTempError) throws AdminException {
    this.counterTempError = counterTempError;
  }

  public void setCounterTempError(int warning, int minor, int major, int critical) throws AdminException {
    setCounterTempError(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpSeverity getSeverityRegister() {
    return severityRegister;
  }

  public void setSeverityRegister(SnmpSeverity severityRegister) throws AdminException {
    this.severityRegister = severityRegister;
  }

  public SnmpSeverity getSeverityRegisterFailed() {
    return severityRegisterFailed;
  }

  public void setSeverityRegisterFailed(SnmpSeverity severityRegisterFailed) throws AdminException {
    this.severityRegisterFailed = severityRegisterFailed;

  }

  public SnmpSeverity getSeverityUnregister() {
    return severityUnregister;
  }

  public void setSeverityUnregister(SnmpSeverity severityUnregister) throws AdminException {
    this.severityUnregister = severityUnregister;
  }

  public SnmpSeverity getSeverityUnregisterFailed() {
    return severityUnregisterFailed;
  }

  public void setSeverityUnregisterFailed(SnmpSeverity severityUnregisterFailed) throws AdminException {
    this.severityUnregisterFailed = severityUnregisterFailed;

  }
  
  private boolean _equals(SnmpCounter v1, SnmpCounter v2) {
    if ((v1 == null) !=  (v2 == null))
      return false;
    if (v1 != null && v2 != null)
      return v1.equals(v2);
    return false; // v1== null && v2 == null
  }
  
  public boolean equals(Object o) {
    if (!(o instanceof SnmpObject))
      return false;
    
    SnmpObject s = (SnmpObject)o;
    if (s.enabled != enabled)
      return false;
    if (!_equals(s.counter000d,counter000d))
      return false;
    if (!_equals(s.counter0014,counter0014))
      return false;
    if (!_equals(s.counter0058,counter0058))
      return false;
    if (!_equals(s.counterAccepted,counterAccepted))
      return false;
    if (!_equals(s.counterDelivered,counterDelivered))
      return false;
    if (!_equals(s.counterFailed,counterFailed))
      return false;
    if (!_equals(s.counterOther,counterOther))
      return false;
    if (!_equals(s.counterRejected,counterRejected))
      return false;
    if (!_equals(s.counterRetried,counterRetried))
      return false;
    if (!_equals(s.counterSDP,counterSDP))
      return false;
    if (!_equals(s.counterTempError,counterTempError))
      return false;
    return true;
  }
}
