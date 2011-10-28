package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class SnmpException extends AdminException {

  SnmpException(String key) {
    super(key);
  }

}
