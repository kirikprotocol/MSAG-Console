package ru.novosoft.smsc.web.controllers.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.snmp.SnmpCounter;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 29.09.2010
 * Time: 18:29:10
 */
public class SnmpCounterWrapper {
  private SnmpController controller;
  private SnmpCounter counter;
  public SnmpCounterWrapper(SnmpController controller, SnmpCounter counter) {
    this.controller = controller;
    this.counter    = counter;
  }

  public int getWarning() {
    return counter.getWarning();
  }

  public int getMinor() {
    return counter.getMinor();
  }

  public int getMajor() {
    return counter.getMajor();
  }

  public int getCritical() {
    return counter.getCritical();
  }

  public void setWarning(int warning) throws AdminException {
    counter.setWarning(warning);
    controller.setSettings();
  }

  public void setMinor(int minor) throws AdminException {
    counter.setMinor(minor);
    controller.setSettings();
  }

  public void setMajor(int major) throws AdminException {
    counter.setMajor(major);
    controller.setSettings();
  }

  public void setCritical(int critical) throws AdminException {
    counter.setCritical(critical);
    controller.setSettings();
  }

  @Override
  public boolean equals(Object o) {
    return counter.equals(o);
  }
}
