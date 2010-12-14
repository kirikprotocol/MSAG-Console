package ru.novosoft.smsc.admin.snmp;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class SnmpFilter {

  public static enum Type {ALL, OPENED, CLOSED}

  private Type type;

  private Date from;

  private Date till;

  public SnmpFilter(Type type) {
    this.type = type;
  }

  public Type getType() {
    return type;
  }

  public Date getFrom() {
    return from;
  }

  public void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  public void setTill(Date till) {
    this.till = till;
  }
}
