package ru.novosoft.smsc.admin.snmp;

/**
 * Структура, описывающая SnmpCounter
 * @author Artem Snopkov
 */
public final class SnmpCounter {

  private final int warning;
  private final int minor;
  private final int major;
  private final int critical;

  public SnmpCounter(int warning, int minor, int major, int critical) {
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

  public boolean equals(Object o) {
    if (o instanceof SnmpCounter) {
      SnmpCounter c = (SnmpCounter)o;
      return warning == c.warning && minor == c.minor && major == c.major && critical == c.critical;
    } else
      return false;
  }
}
