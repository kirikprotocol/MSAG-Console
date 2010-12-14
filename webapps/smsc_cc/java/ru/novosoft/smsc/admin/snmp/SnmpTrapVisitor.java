package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public interface SnmpTrapVisitor {

  public boolean visit(SnmpTrap r) throws AdminException;
}
