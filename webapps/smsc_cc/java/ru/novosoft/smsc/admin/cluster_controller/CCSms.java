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

  @Override
  public String toString() {
    final StringBuilder sb = new StringBuilder();
    sb.append("CCSms");
    sb.append("{id='").append(id).append('\'');
    sb.append(", srcAddress=").append(srcAddress);
    sb.append(", dstAddress=").append(dstAddress);
    sb.append('}');
    return sb.toString();
  }
}
