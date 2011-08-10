package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

import java.util.LinkedList;
import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWRoute {

  private static final ValidationHelper vh = new ValidationHelper(SmppGWRoute.class);

  private int deliveryId;

  private String user;

  private final List<Address> serviceNumbers = new LinkedList<Address>();

  public SmppGWRoute() {
  }

  public SmppGWRoute(SmppGWRoute route) {
    this.deliveryId = route.deliveryId;
    for(Address a : route.serviceNumbers) {
      this.serviceNumbers.add(new Address(a));
    }
    this.user = route.user;
  }

  public int getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(int deliveryId) {
    this.deliveryId = deliveryId;
  }

  public void addServiceNumber(Address a) {
    if(a != null && !serviceNumbers.contains(a)) {
      serviceNumbers.add(a);
    }
  }

  public void removeServiceNumber(String a) {
    serviceNumbers.remove(new Address(a));
  }

  public void clearServiceNumbers() {
    serviceNumbers.clear();
  }

  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    this.user = user;
  }

  public List<Address> getServiceNumbers() {
    return serviceNumbers;
  }

  public void validate() throws AdminException {
    vh.checkGreaterOrEqualsTo("deliveryId", deliveryId, 0);
    vh.checkNotEmpty("user", user);
    vh.checkSizeGreaterThan("serviceNumbers",serviceNumbers, 0);
  }


  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    SmppGWRoute that = (SmppGWRoute) o;

    if (deliveryId != that.deliveryId) return false;

    if(serviceNumbers.size() != that.serviceNumbers.size()) {
      return false;
    }

    for(Address a : serviceNumbers) {
      if(!that.serviceNumbers.contains(a)) {
        return false;
      }
    }
    if (user != null ? !user.equals(that.user) : that.user != null) return false;

    return true;
  }

}
