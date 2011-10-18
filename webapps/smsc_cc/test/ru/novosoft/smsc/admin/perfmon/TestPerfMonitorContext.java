package ru.novosoft.smsc.admin.perfmon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.InetAddress;

/**
 * author: Aleksandr Khalitov
 */
public class TestPerfMonitorContext implements PerfMonitorContext{

  private final int[] ports;

  public TestPerfMonitorContext(int[] ports) {
    this.ports = ports;
  }

  public InetAddress getPerfMonitorAddress(int instance) throws AdminException {
    return null;
  }

  public int getPerfMonitorCount() throws AdminException {
    return ports.length;
  }

  public int getAppletPort(int instance) throws AdminException {
    return ports[instance];
  }

  public boolean isSupport64Bit() throws AdminException {
    return false;
  }
}
