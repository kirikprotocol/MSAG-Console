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
      this.endpoints.put(e.getSystemId(), new SmppGWEndpoint(e));
    }
  }

  public void validate() throws AdminException {
    Set<String> usedNames = new HashSet<String>(10);
    for(SmppGWEndpoint e : endpoints.values()) {
      e.validate();
      if(usedNames.contains(e.getName())) {     //Проверяем, что name уникален
        throw new SmppGWException("endpoint_name_intersection", e.getName());
      }
      usedNames.add(e.getName());
    }
  }

  public void addEndpoint(SmppGWEndpoint endpoint) throws AdminException{
    endpoint.validate();
    if(endpoints.containsKey(endpoint.getSystemId())) {
      throw new SmppGWException("system_id_intersection", endpoint.getSystemId());
    }
    endpoints.put(endpoint.getSystemId(), new SmppGWEndpoint(endpoint));
  }

  public void removeEndpoint(String sysId) {
    endpoints.remove(sysId);
  }

  public Collection<SmppGWEndpoint> getEndpoints() {
    Collection<SmppGWEndpoint> rez = new ArrayList<SmppGWEndpoint>(endpoints.size());
    for(SmppGWEndpoint e : endpoints.values()) {
      rez.add(new SmppGWEndpoint(e));
    }
    return rez;
  }

  public SmppGWEndpoint getEndpoint(String sysId) {
    return endpoints.get(sysId);
  }

  public boolean containsEndpoint(String systemId) {
    return endpoints.containsKey(systemId);
  }

  public void save(XmlConfigSection parent) throws XmlConfigException {
    parent.clear();
    for(SmppGWEndpoint e : endpoints.values()) {
      XmlConfigSection section = parent.getOrCreateSection(e.getName());
      section.setString("systemId", e.getSystemId());
      section.setString("password", e.getPassword());
      section.setBool("enabled", e.isEnabled());
    }
  }

  public void load(XmlConfigSection endpoints) throws XmlConfigException, AdminException {
    for(XmlConfigSection es : endpoints.sections()) {
      SmppGWEndpoint e = new SmppGWEndpoint();
      e.setName(es.getName());
      e.setSystemId(es.getString("systemId"));
      e.setPassword(es.getString("password"));
      e.setEnabled(es.getBool("enabled", false));
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
