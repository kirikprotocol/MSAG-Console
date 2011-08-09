package mobi.eyeline.informer.web.controllers.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.smppgw.*;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableRow;
import mobi.eyeline.informer.web.config.Configuration;

import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class ProviderSettingsPage extends ProviderEditPage{

  private String oldName;

  private SmppGWProvider provider;

  private List<String> availableEndpoints = new LinkedList<String>();

  private DynamicTableModel endpoints = new DynamicTableModel();

  private List selected;

  private final boolean isNew;

  ProviderSettingsPage(SmppGWProvider provider, boolean isNew) {
    this.provider = provider;

    Configuration c = getConfig();
    SmppGWProviderSettings providerSettings = c.getSmppGWProviderSettings();
    SmppGWEndpointSettings endpointSettings = c.getSmppGWEndpointSettings();

    if(!isNew) {
      this.oldName = provider.getName();
    }

    for(String e : provider.getEndpoints()) {
      DynamicTableRow r = new DynamicTableRow();
      r.setValue("conn", e);
      endpoints.addRow(r);
    }

    loadAvailableEndpoints(endpointSettings, providerSettings);

    this.isNew = isNew;
  }

  private Route convert(SmppGWRoute r, boolean checked) throws AdminException {
    StringBuilder sb = new StringBuilder();
    boolean first = true;
    for(Address a : r.getServiceNumbers()) {
      if(!first) {
        sb.append(',');
      }
      sb.append(a.getSimpleAddress());
      first = false;
    }
    String adresses = sb.toString();
    String delivery = null;
    Delivery d = getConfig().getDelivery(r.getUser(), r.getDeliveryId());
    if(d != null) {
      delivery = d.getName();
    }
    return new Route(adresses, delivery, r.getDeliveryId(), checked);
  }

  private void loadAvailableEndpoints(SmppGWEndpointSettings endpointSettings, SmppGWProviderSettings providerSettings) {
    Set<String> tmp = new HashSet<String>();
    for(SmppGWEndpoint e : endpointSettings.getEndpoints()) {
      tmp.add(e.getSystemId());
    }
    for(SmppGWProvider p : providerSettings.getProviders()) {
      if(oldName != null && oldName.equals(p.getName())) {
        continue;
      }
      for(String e : p.getEndpoints()) {
        tmp.remove(e);
      }
    }
    availableEndpoints.addAll(tmp);
    Collections.sort(availableEndpoints);
  }

  public DynamicTableModel getEndpoints() {
    return endpoints;
  }

  public void setEndpoints(DynamicTableModel endpoints) {
    this.endpoints = endpoints;
  }

  public List<String> getAvailableEndpoints() {
    return availableEndpoints;
  }

  public List<Route> getRoutes() {
    List<Route> res = new LinkedList<Route>();
    try {
      for(SmppGWRoute r : provider.getRoutes()) {
        res.add(convert(r, selected != null && selected.contains(Integer.toString(r.getDeliveryId()))));
      }
    } catch (AdminException e) {
      addError(e);
    }
    return res;
  }

  public List getSelected() {
    return selected;
  }

  public void setSelected(List selected) {
    this.selected = selected;
  }

  public String getOldName() {
    return oldName;
  }

  public void setOldName(String oldName) {
    this.oldName = oldName;
  }

  public SmppGWProvider getProvider() {
    return provider;
  }

  public void setProvider(SmppGWProvider provider) {
    this.provider = provider;
  }

  public String removeRoutes() {
    if(selected != null && !selected.isEmpty()) {
      for(String s : (List<String>)selected) {
        provider.removeRoute(Integer.parseInt(s));
      }
      selected.clear();
    }
    return null;
  }

  public static class Route {

    private final String addresses;
    private final String delivery;
    private final boolean selected;
    private final int deliveryId;

    public Route(String addresses, String delivery, int deliveryId, boolean selected) {
      this.addresses = addresses;
      this.delivery = delivery;
      this.deliveryId = deliveryId;
      this.selected = selected;
    }

    public int getDeliveryId() {
      return deliveryId;
    }

    public boolean isSelected() {
      return selected;
    }

    public String getAddresses() {
      return addresses;
    }

    public String getDelivery() {
      return delivery;
    }
  }

  private void setEndpoints() {
    provider.clearEndpoints();
    for(DynamicTableRow r : endpoints.getRows()) {
      provider.addEndpoint((String)r.getValue("conn"));
    }
  }

  @Override
  public ProviderEditPage nextPage() throws AdminException{
    setEndpoints();
    provider.validate();
    String r = getRequestParameter("deliveryId");
    if(r != null && r.length()>0) {
      return new RouteSettingsPage(provider, Integer.parseInt(r), isNew);
    }
    return new ChooseDeliveryPage(provider, isNew);
  }

  public String save() {
    setEndpoints();
    SmppGWProviderSettings ss = getConfig().getSmppGWProviderSettings();
    if(!isNew) {
      ss.removeProvider(oldName);
    }
    try{
      ss.addProvider(provider);
      getConfig().updateSmppGWSettings(ss);
    }catch (AdminException e) {
      addError(e);
      return null;
    }
    return "SMPPGW_PROVIDERS";
  }


  @Override
  public ProviderEditPage backPage() throws AdminException {
    return null;
  }

  @Override
  public String getPageId() {
    return "EDIT_SETTINGS";
  }

  public boolean isNew() {
    return isNew;
  }
}
