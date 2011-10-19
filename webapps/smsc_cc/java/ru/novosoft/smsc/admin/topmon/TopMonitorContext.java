package ru.novosoft.smsc.admin.topmon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.InetAddress;

/**
 * author: Aleksandr Khalitov
 */
public interface TopMonitorContext {

  public InetAddress getTopMonitorAddress(int instance) throws AdminException;

  public int getTopMonitorCount() throws AdminException;

  public int getAppletPort(int instance) throws AdminException;

}
