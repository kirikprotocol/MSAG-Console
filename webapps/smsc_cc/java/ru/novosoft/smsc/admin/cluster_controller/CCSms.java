package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.util.Address;

/**
 * author: Aleksandr Khalitov
 */
public class CCSms {

  private final String id;
  private final Address srcAddress;
  private final Address dstAddress;

  public CCSms(String id, Address srcAddress, Address dstAddress) {
    this.id = id;
    this.srcAddress = srcAddress;
    this.dstAddress = dstAddress;
  }

  public String getId() {
    return id;
  }

  public Address getSrcAddress() {
    return srcAddress;
  }

  public Address getDstAddress() {
    return dstAddress;
  }
}
