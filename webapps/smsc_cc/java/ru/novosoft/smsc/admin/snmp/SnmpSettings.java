package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class SnmpSettings {

  private static final ValidationHelper vh = new ValidationHelper(SnmpSettings.class);

  private int counterInterval;

  private SnmpObject defaultSnmpObject;
  private Map<String, SnmpObject> snmpObjects = new HashMap<String, SnmpObject>();

  SnmpSettings() {
  }

  SnmpSettings(SnmpSettings c) {
    this.counterInterval = c.counterInterval;
    this.defaultSnmpObject = new SnmpObject(c.defaultSnmpObject);
    this.snmpObjects = new HashMap<String, SnmpObject>();
    for (Map.Entry<String, SnmpObject> e : c.snmpObjects.entrySet())
      this.snmpObjects.put(e.getKey(), new SnmpObject(e.getValue()));
  }

  public int getCounterInterval() {
    return counterInterval;
  }

  public void setCounterInterval(int counterInterval) throws AdminException {
    vh.checkPositive("counterInterval", counterInterval);
    this.counterInterval = counterInterval;
  }

  public SnmpObject getDefaultSnmpObject() {
    return defaultSnmpObject;
  }

  public void setDefaultSnmpObject(SnmpObject defaultSnmpObject) {    
    this.defaultSnmpObject = defaultSnmpObject;
  }

  public Map<String, SnmpObject> getSnmpObjects() {
    return snmpObjects;
  }

  public void setSnmpObjects(Map<String, SnmpObject> snmpObjects) throws AdminException {
    vh.checkNoNulls("snmpObjects", snmpObjects);
    this.snmpObjects = snmpObjects;
  }
}
