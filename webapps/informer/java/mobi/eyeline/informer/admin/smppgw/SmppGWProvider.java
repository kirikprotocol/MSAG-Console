package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;

import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWProvider {

  private static final ValidationHelper vh = new ValidationHelper(SmppGWProvider.class);

  private String name;

  private String descr;

  private List<String> endpoints = new LinkedList<String>();

  private Map<Integer, SmppGWRoute> routes = new HashMap<Integer, SmppGWRoute>();

  public SmppGWProvider() {
  }

  public SmppGWProvider(SmppGWProvider p) {
    this.name = p.name;
    this.descr = p.descr;
    this.endpoints.addAll(p.endpoints);
    for(SmppGWRoute r : p.routes.values()) {
      this.routes.put(r.getDeliveryId(), new SmppGWRoute(r));
    }
  }

  public void addEndpoint(String endpoint) {
    if(endpoint != null && !endpoints.contains(endpoint)) {
      endpoints.add(endpoint);
    }
  }

  public void clearEndpoints() {
    endpoints.clear();
  }

  public void clearRoutes() {
    routes.clear();
  }

  public void removeEndpoint(String name) {
    endpoints.remove(name);
  }

  public List<String> getEndpoints() {
    return new ArrayList<String>(endpoints);
  }


  public void addRoute(SmppGWRoute route) throws AdminException{
    route.validate();
    if(routes.containsKey(route.getDeliveryId())) {
      throw new SmppGWException("route_delivery_intersection", Integer.toString(route.getDeliveryId()));
    }
    routes.put(route.getDeliveryId(), new SmppGWRoute(route));
  }

  public void removeRoute(int deliveryId) {
    routes.remove(deliveryId);
  }

  public List<SmppGWRoute> getRoutes() {
    List<SmppGWRoute> res = new ArrayList<SmppGWRoute>(routes.size());
    for(SmppGWRoute r : routes.values()) {
      res.add(r);
    }
    return res;
  }

  public SmppGWRoute getRoute(int deliveryId) {
    for(SmppGWRoute r : routes.values()) {
      if(r.getDeliveryId() == deliveryId) {
        return r;
      }
    }
    return null;
  }

  public void validate() throws AdminException {
    vh.checkNotEmpty("name", name);
    if(routes.isEmpty()) {
      return;
    }
    Set<Address> usedAddresses = new HashSet<Address>(10);
    for(SmppGWRoute r : routes.values()) {
      for(Address a : r.getServiceNumbers()) {
        if(usedAddresses.contains(a)) {      // Проверяем, что каждый в маршруте уникален для данного КП
          throw new SmppGWException("service_address_intersection", a.getSimpleAddress());
        }
        usedAddresses.add(a);
      }
    }
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getDescr() {
    return descr;
  }

  public void setDescr(String descr) {
    this.descr = descr;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    SmppGWProvider that = (SmppGWProvider) o;

    if (descr != null ? !descr.equals(that.descr) : that.descr != null) return false;
    if (name != null ? !name.equals(that.name) : that.name != null) return false;

    if(endpoints.size() != that.endpoints.size()) {
      return false;
    }
    for(String e : endpoints) {
      if(!that.endpoints.contains(e)) {
        return false;
      }
    }

    if(routes.size() != that.routes.size()) {
      return false;
    }
    for(Map.Entry<Integer, SmppGWRoute> e : routes.entrySet()) {
      SmppGWRoute r = that.routes.get(e.getKey());
      if(r == null || !r.equals(e.getValue())) {
        return false;
      }
    }

    return true;
  }

}
