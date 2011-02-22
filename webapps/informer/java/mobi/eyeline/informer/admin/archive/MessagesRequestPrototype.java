package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.util.Address;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class MessagesRequestPrototype {

  private String name;

  private Date from;

  private Date till;

  private Address address;

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public Date getFrom() {
    return from;
  }

  public void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  public void setTill(Date till) {
    this.till = till;
  }

  public Address getAddress() {
    return address;
  }

  public void setAddress(Address address) {
    this.address = address;
  }
}
