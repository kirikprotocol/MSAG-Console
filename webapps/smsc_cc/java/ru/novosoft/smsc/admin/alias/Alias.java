package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.util.Address;

/**
 * Структура, описывающая алиас
 * @author Artem Snopkov
 */
public class Alias {

  private final Address address;
	private final Address alias;
	private final Boolean hide;

  public Alias(Address address, Address alias, boolean hide) {
    this.address = address;
    this.alias = alias;
    this.hide = hide;
  }    

  public Address getAddress() {
    return address;
  }

  public Address getAlias() {
    return alias;
  }

  public boolean isHide() {
    return hide;
  }

  @Override
  public String toString() {
    return "Alias{" +
        "address=" + address +
        ", alias=" + alias +
        ", hide=" + hide +
        '}';
  }
}
