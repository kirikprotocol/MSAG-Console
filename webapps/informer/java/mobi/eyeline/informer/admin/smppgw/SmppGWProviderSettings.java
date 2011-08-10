package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWProviderSettings {

  private final Map<String, SmppGWProvider> providers = new HashMap<String, SmppGWProvider>();

  public SmppGWProviderSettings() {
  }

  public SmppGWProviderSettings(SmppGWProviderSettings s) {
    for(SmppGWProvider p : s.providers.values()) {
      this.providers.put(p.getName(), new SmppGWProvider(p));
    }
  }


  public void validate() throws AdminException {
    Set<String> usedSystemIds = new HashSet<String>(10);
    for(SmppGWProvider p : providers.values()) {
      p.validate();
      for(String e : p.getEndpoints()) {
        if(usedSystemIds.contains(e)) {        // Проверяем, что каждое соединение используется только один раз
          throw new SmppGWException("endpoint_intersection", e);
        }
        usedSystemIds.add(e);
      }
    }
  }

  public void addProvider(SmppGWProvider provider) throws AdminException{
    provider.validate();
    if(providers.containsKey(provider.getName())) {
      throw new SmppGWException("provider_name_intersection", provider.getName());
    }
    providers.put(provider.getName(), new SmppGWProvider(provider));
  }

  public void removeProvider(String name) {
    providers.remove(name);
  }

  public Collection<SmppGWProvider> getProviders() {
    Collection<SmppGWProvider> rez = new ArrayList<SmppGWProvider>(providers.size());
    for(SmppGWProvider e : providers.values()) {
      rez.add(e);
    }
    return rez;
  }

  public SmppGWProvider getProvider(String name) {
    return providers.get(name);
  }

  public SmppGWProvider getProviderByEndpoint(String endpointSystemId) {
    for(SmppGWProvider p : providers.values()) {
      if(p.getEndpoints().contains(endpointSystemId)) {
        return p;
      }
    }
    return null;
  }

  public void load(XmlConfigSection providers) throws XmlConfigException, AdminException {
    for(XmlConfigSection ps : providers.sections()) {
      SmppGWProvider p = new SmppGWProvider();
      p.setName(ps.getName());
      p.setDescr(ps.getString("description", null));
      if(ps.containsParam("endpoint_ids")) {
        for(String e : ps.getStringArray("endpoint_ids", ",")) {
          p.addEndpoint(e);
        }
      }
      if(ps.containsSection("deliveries")) {
        for(XmlConfigSection s : ps.getSection("deliveries").sections()) {
          SmppGWRoute route = new SmppGWRoute();
          route.setDeliveryId(Integer.parseInt(s.getName()));
          route.setUser(s.getString("user"));
          if(s.containsParam("services_numbers")) {
            for(String n : s.getStringArray("services_numbers",",")) {
              route.addServiceNumber(new Address(n));
            }
          }
          p.addRoute(route);
        }
      }
      addProvider(p);
    }
  }

  public void save(XmlConfigSection parent) throws XmlConfigException {
    parent.clear();
    for(SmppGWProvider p : providers.values()) {
      XmlConfigSection section = parent.getOrCreateSection(p.getName());
      if(p.getDescr() != null) {
        section.setString("description", p.getDescr());
      }
      section.setStringList("endpoint_ids", p.getEndpoints(), ",");
      XmlConfigSection routesSection = section.getOrCreateSection("deliveries");
      for(SmppGWRoute r : p.getRoutes()) {
        XmlConfigSection rs = routesSection.getOrCreateSection(Integer.toString(r.getDeliveryId()));
        rs.setString("user", r.getUser());
        List<String> sn = new LinkedList<String>();
        for(Address a : r.getServiceNumbers()) {
          sn.add(a.getSimpleAddress());
        }
        rs.setStringList("services_numbers", sn , ",");
      }
    }
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    SmppGWProviderSettings that = (SmppGWProviderSettings) o;

    if(providers.size() != that.providers.size()) {
      return false;
    }

    for(Map.Entry<String, SmppGWProvider> e : providers.entrySet()) {
      if(!e.getValue().equals(that.providers.get(e.getKey()))) {
        return false;
      }
    }

    return true;
  }
}
