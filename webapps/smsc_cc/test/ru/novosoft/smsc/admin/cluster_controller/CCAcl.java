package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * @author Artem Snopkov
 */
class CCAcl {

  private CCAclInfo info;
  private final Set<Address> addresses = new HashSet<Address>();

  CCAcl(CCAclInfo info) {
    this(info, new ArrayList<Address>());
  }

  CCAcl(CCAclInfo info, List<Address> addresses) {
    this.info = info;
    this.addresses.addAll(addresses);
  }

  CCAcl(CCAclInfo info, String ... addresses) {
    this.info = info;
    for (String addr : addresses)
      this.addresses.add(new Address(addr));
  }

  public void updateInfo(CCAclInfo info) {
    this.info = info;
  }

  public CCAclInfo getInfo() {
    return info;
  }

  public List<Address> getAddresses() {
    return new ArrayList<Address>(addresses);
  }

  public void addAddresses(List<Address> addresses) {
    this.addresses.addAll(addresses);
  }

  public void removeAddresses(List<Address> addresses) {
    this.addresses.removeAll(addresses);
  }
}
