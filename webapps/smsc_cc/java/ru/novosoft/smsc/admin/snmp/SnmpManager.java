package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;

/**
 * @author Artem Snopkov
 */
public interface SnmpManager extends SmscConfiguration {

  SnmpSettings getSettings() throws AdminException;

  void updateSettings(SnmpSettings settings) throws AdminException;

  void getTraps(SnmpFilter filter, SnmpTrapVisitor visitor) throws AdminException;
}
