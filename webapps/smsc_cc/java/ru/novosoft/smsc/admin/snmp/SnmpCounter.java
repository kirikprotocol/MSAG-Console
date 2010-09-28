package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

/**
 * Структура, описывающая SnmpCounter
 * @author Artem Snopkov
 */
public final class SnmpCounter {

  private static final ValidationHelper vh = new ValidationHelper(SnmpCounter.class.getCanonicalName());

  private int warning;
  private int minor;
  private int major;
  private int critical;

  public SnmpCounter(int warning, int minor, int major, int critical) throws AdminException {
    vh.checkGreaterOrEqualsTo("warning", warning, 0);
    vh.checkGreaterOrEqualsTo("minor", minor, 0);
    vh.checkGreaterOrEqualsTo("major", major, 0);
    vh.checkGreaterOrEqualsTo("critical", critical, 0);
    this.warning = warning;
    this.minor = minor;
    this.major = major;
    this.critical = critical;
  }

  public SnmpCounter(SnmpCounter counter) {
    this.warning = counter.warning;
    this.minor = counter.minor;
    this.major = counter.major;
    this.critical = counter.critical;
  }

  public int getWarning() {
    return warning;
  }

  public int getMinor() {
    return minor;
  }

  public int getMajor() {
    return major;
  }

  public int getCritical() {
    return critical;
  }


  public void setWarning(int warning) throws AdminException {
    vh.checkGreaterOrEqualsTo("warning", warning, 0);
    this.warning = warning;
  }

  public void setMinor(int minor) throws AdminException {
    vh.checkGreaterOrEqualsTo("minor", minor, 0);
    this.minor = minor;
  }

  public void setMajor(int major) throws AdminException {
    vh.checkGreaterOrEqualsTo("major", major, 0);
    this.major = major;
  }

  public void setCritical(int critical) throws AdminException {
    vh.checkGreaterOrEqualsTo("critical", critical, 0);
    this.critical = critical;
  }

  public boolean equals(Object o) {
    if (o instanceof SnmpCounter) {
      SnmpCounter c = (SnmpCounter)o;
      return warning == c.warning && minor == c.minor && major == c.major && critical == c.critical;
    } else
      return false;
  }
}
