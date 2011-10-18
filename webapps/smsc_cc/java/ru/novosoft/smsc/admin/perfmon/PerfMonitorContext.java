package ru.novosoft.smsc.admin.perfmon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.InetAddress;

/**
 * author: Aleksandr Khalitov
 */
public interface PerfMonitorContext {

  public InetAddress getPerfMonitorAddress(int instance) throws AdminException;

  public int getPerfMonitorCount() throws AdminException;

  public int getAppletPort(int instance) throws AdminException;

  public boolean isSupport64Bit() throws AdminException;

}
