package ru.novosoft.smsc.admin.snmp;

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

  public SnmpObject setCounterOther(SnmpCounter counterOther) {
    this.counterOther = counterOther;
    return this;
  }

  public SnmpObject setCounterOther(int warning, int minor, int major, int critical) {
    return setCounterOther(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounter0014() {
    return counter0014;
  }

  public SnmpObject setCounter0014(SnmpCounter counter0014) {
    this.counter0014 = counter0014;
    return this;
  }

  public SnmpObject setCounter0014(int warning, int minor, int major, int critical) {
    return setCounter0014(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounter000d() {
    return counter000d;
  }

  public SnmpObject setCounter000d(SnmpCounter counter000d) {
    this.counter000d = counter000d;
    return this;
  }

  public SnmpObject setCounter000d(int warning, int minor, int major, int critical) {
    return setCounter000d(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterFailed() {
    return counterFailed;
  }

  public SnmpObject setCounterFailed(SnmpCounter counterFailed) {
    this.counterFailed = counterFailed;
    return this;
  }

  public SnmpObject setCounterFailed(int warning, int minor, int major, int critical) {
    return setCounterFailed(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterAccepted() {
    return counterAccepted;
  }

  public SnmpObject setCounterAccepted(SnmpCounter counterAccepted) {
    this.counterAccepted = counterAccepted;
    return this;
  }

  public SnmpObject setCounterAccepted(int warning, int minor, int major, int critical) {
    return setCounterAccepted(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterRejected() {
    return counterRejected;
  }

  public SnmpObject setCounterRejected(SnmpCounter counterRejected) {
    this.counterRejected = counterRejected;
    return this;
  }

  public SnmpObject setCounterRejected(int warning, int minor, int major, int critical) {
    return setCounterRejected(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterDelivered() {
    return counterDelivered;
  }

  public SnmpObject setCounterDelivered(SnmpCounter counterDelivered) {
    this.counterDelivered = counterDelivered;
    return this;
  }

  public SnmpObject setCounterDelivered(int warning, int minor, int major, int critical) {
    return setCounterDelivered(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterSDP() {
    return counterSDP;
  }

  public SnmpObject setCounterSDP(SnmpCounter counterSDP) {
    this.counterSDP = counterSDP;
    return this;
  }

  public SnmpObject setCounterSDP(int warning, int minor, int major, int critical) {
    return setCounterSDP(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterRetried() {
    return counterRetried;
  }

  public SnmpObject setCounterRetried(SnmpCounter counterRetried) {
    this.counterRetried = counterRetried;
    return this;
  }

  public SnmpObject setCounterRetried(int warning, int minor, int major, int critical) {
    return setCounterRetried(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounter0058() {
    return counter0058;
  }

  public SnmpObject setCounter0058(SnmpCounter counter0058) {
    this.counter0058 = counter0058;
    return this;
  }

  public SnmpObject setCounter0058(int warning, int minor, int major, int critical) {
    return setCounter0058(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpCounter getCounterTempError() {
    return counterTempError;
  }

  public SnmpObject setCounterTempError(SnmpCounter counterTempError) {
    this.counterTempError = counterTempError;
    return this;
  }

  public SnmpObject setCounterTempError(int warning, int minor, int major, int critical) {
    return setCounterTempError(new SnmpCounter(warning, minor, major, critical));
  }

  public SnmpSeverity getSeverityRegister() {
    return severityRegister;
  }

  public SnmpObject setSeverityRegister(SnmpSeverity severityRegister) {
    this.severityRegister = severityRegister;
    return this;
  }

  public SnmpSeverity getSeverityRegisterFailed() {
    return severityRegisterFailed;
  }

  public SnmpObject setSeverityRegisterFailed(SnmpSeverity severityRegisterFailed) {
    this.severityRegisterFailed = severityRegisterFailed;
    return this;
  }

  public SnmpSeverity getSeverityUnregister() {
    return severityUnregister;
  }

  public SnmpObject setSeverityUnregister(SnmpSeverity severityUnregister) {
    this.severityUnregister = severityUnregister;
    return this;
  }

  public SnmpSeverity getSeverityUnregisterFailed() {
    return severityUnregisterFailed;
  }

  public SnmpObject setSeverityUnregisterFailed(SnmpSeverity severityUnregisterFailed) {
    this.severityUnregisterFailed = severityUnregisterFailed;
    return this;
  }
}
