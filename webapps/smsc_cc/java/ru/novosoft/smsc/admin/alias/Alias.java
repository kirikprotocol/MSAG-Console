package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.util.ValidationException;
import ru.novosoft.smsc.util.Address;

/**
 * Структура, описывающая алиас
 * @author Artem Snopkov
 */
public class Alias {

  private final Address address;
	private final Address alias;
	private final Boolean hide;

  Alias(String address, String alias, boolean hide) {
    this.address = new Address(address);
    this.alias = new Address(alias);
    this.hide = hide;
  }

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

  public boolean equals(Object o) {
    if (o instanceof Alias) {
      Alias a = (Alias)o;
      return a.alias.equals(alias);
    }
    return false;
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
