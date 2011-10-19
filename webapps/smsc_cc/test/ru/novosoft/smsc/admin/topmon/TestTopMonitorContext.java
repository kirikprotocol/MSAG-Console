package ru.novosoft.smsc.admin.topmon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.InetAddress;

/**
* author: Aleksandr Khalitov
*/
class TestTopMonitorContext implements TopMonitorContext {
  private final int[] ports;

  public TestTopMonitorContext(int[] ports) {
    this.ports = ports;
  }

  public InetAddress getTopMonitorAddress(int instance) throws AdminException {
    return null;
  }

  public int getTopMonitorCount() throws AdminException {
    return ports.length;
  }

  public int getAppletPort(int instance) throws AdminException {
    try{
      return ports[instance];
    }catch (IndexOutOfBoundsException e){
      return -1;
    }
  }
}
