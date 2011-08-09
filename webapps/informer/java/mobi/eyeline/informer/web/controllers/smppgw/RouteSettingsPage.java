package mobi.eyeline.informer.web.controllers.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.smppgw.SmppGWProvider;
import mobi.eyeline.informer.admin.smppgw.SmppGWRoute;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableRow;

import javax.faces.application.FacesMessage;
import java.util.HashSet;
import java.util.Set;

/**
 * author: Aleksandr Khalitov
 */


public class RouteSettingsPage extends ProviderEditPage{

  private SmppGWProvider provider;

  private String deliveryName;

  private DynamicTableModel serviceNumbers = new DynamicTableModel();

  private SmppGWRoute route;

  private final boolean isNew;

  RouteSettingsPage(SmppGWProvider provider, int deliveryId, boolean aNew) throws AdminException{
    this.provider = provider;
    isNew = aNew;
    Delivery d = getConfig().getDelivery(getUserName(), deliveryId);
    if(d != null) {
      deliveryName = d.getName();
    }else {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "delivery.not.found", Integer.toString(deliveryId));
      return;
    }
    route = provider.getRoute(deliveryId);
    if(route == null) {
      route = new SmppGWRoute();
      route.setUser(d.getOwner());
      route.setDeliveryId(deliveryId);
    }else {
      for(Address a : route.getServiceNumbers()) {
        DynamicTableRow row = new DynamicTableRow();
        row.setValue("address", a.getSimpleAddress());
        serviceNumbers.addRow(row);
      }
    }
  }

  @Override
  public ProviderEditPage nextPage() throws AdminException {
    if(serviceNumbers.getRowCount() == 0) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smppgw.provider.edit.route.serrviceNumbers.empty");
      return null;
    }
    Set<Address> addresses = new HashSet<Address>(serviceNumbers.getRowCount());
    for(DynamicTableRow r : serviceNumbers.getRows()) {
      String a = (String)r.getValue("address");
      if(a.length() == 0) {
        continue;
      }
      if(!Address.validate(a)) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smppgw.provider.edit.route.serrviceNumbers.illegal", a);
        return null;
      }
      addresses.add(new Address(a));
    }
    for(SmppGWRoute r : provider.getRoutes()) {
      if(r.getDeliveryId() == route.getDeliveryId()) {
        continue;
      }
      for(Address a : r.getServiceNumbers()) {
        if(addresses.contains(a)) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smppgw.provider.edit.route.serrviceNumbers.intersection", a.getSimpleAddress());
          return null;
        }
      }
    }
    route.clearServiceNumbers();
    for(Address a : addresses) {
      route.addServiceNumber(a);
    }
    route.validate();

    provider.removeRoute(route.getDeliveryId());
    provider.addRoute(route);

    return new ProviderSettingsPage(provider, isNew);
  }

  @Override
  public ProviderEditPage backPage() throws AdminException {
    return new ChooseDeliveryPage(provider, isNew);
  }

  @Override
  public String getPageId() {
    return "ROUTE_SETTINGS";
  }

  public String getDeliveryName() {
    return deliveryName;
  }

  public SmppGWRoute getRoute() {
    return route;
  }

  public void setRoute(SmppGWRoute route) {
    this.route = route;
  }

  public DynamicTableModel getServiceNumbers() {
    return serviceNumbers;
  }

  public void setServiceNumbers(DynamicTableModel serviceNumbers) {
    this.serviceNumbers = serviceNumbers;
  }
}
