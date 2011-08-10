package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWEndpointSettings {

  private final Map<String, SmppGWEndpoint> endpoints = new HashMap<String, SmppGWEndpoint>();

  public SmppGWEndpointSettings() {
  }

  public SmppGWEndpointSettings(SmppGWEndpointSettings s) {
    for(SmppGWEndpoint e : s.endpoints.values()) {
      this.endpoints.put(e.getName(), new SmppGWEndpoint(e));
    }
  }

  public void validate() throws AdminException {
    Set<String> usedSysIds = new HashSet<String>(10);
    for(SmppGWEndpoint e : endpoints.values()) {
      e.validate();
      if(usedSysIds.contains(e.getSystemId())) {     //Проверяем, что systemId уникален
        throw new SmppGWException("system_id_intersection", e.getSystemId());
      }
      usedSysIds.add(e.getSystemId());
    }
  }

  public void addEndpoint(SmppGWEndpoint endpoint) throws AdminException{
    endpoint.validate();
    if(endpoints.containsKey(endpoint.getName())) {
      throw new SmppGWException("endpoint_name_intersection", endpoint.getName());
    }
    endpoints.put(endpoint.getName(), new SmppGWEndpoint(endpoint));
  }

  public void removeEndpoint(String name) {
    endpoints.remove(name);
  }

  public Collection<SmppGWEndpoint> getEndpoints() {
    Collection<SmppGWEndpoint> rez = new ArrayList<SmppGWEndpoint>(endpoints.size());
    for(SmppGWEndpoint e : endpoints.values()) {
      rez.add(new SmppGWEndpoint(e));
    }
    return rez;
  }

  public SmppGWEndpoint getEndpoint(String name) {
    return endpoints.get(name);
  }

  public boolean containsEndpoint(String systemId) {
    for(SmppGWEndpoint e :  endpoints.values()) {
      if(e.getSystemId().equals(systemId)) {
        return true;
      }
    }
    return false;
  }

  public void save(XmlConfigSection parent) throws XmlConfigException {
    parent.clear();
    for(SmppGWEndpoint e : endpoints.values()) {
      XmlConfigSection section = parent.getOrCreateSection(e.getName());
      section.setString("systemId", e.getSystemId());
      section.setString("password", e.getPassword());
    }
  }

  public void load(XmlConfigSection endpoints) throws XmlConfigException, AdminException {
    for(XmlConfigSection es : endpoints.sections()) {
      SmppGWEndpoint e = new SmppGWEndpoint();
      e.setName(es.getName());
      e.setSystemId(es.getString("systemId"));
      e.setPassword(es.getString("password"));
      addEndpoint(e);
    }
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    SmppGWEndpointSettings that = (SmppGWEndpointSettings) o;

    if(endpoints.size() != that.endpoints.size()) {
      return false;
    }

    for(Map.Entry<String, SmppGWEndpoint> e : endpoints.entrySet()) {
      if(!e.getValue().equals(that.endpoints.get(e.getKey()))) {
        return false;
      }
    }

    return true;
  }

}
