package ru.novosoft.smsc.admin.network_profiles;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class NetworkProfile {

  private Map masks = new HashMap(10);

  private String ussdOpenDestRef;

  private String abonentStatusMethod;

  private String name;

  public NetworkProfile(String name) {
    this.name = name;
  }

  public Map getMasks() {
    return masks;
  }

  public void setMasks(Map masks) {
    this.masks = masks;
  }

  public String getUssdOpenDestRef() {
    return ussdOpenDestRef;
  }

  public void setUssdOpenDestRef(String ussdOpenDestRef) {
    this.ussdOpenDestRef = ussdOpenDestRef;
  }

  public String getAbonentStatusMethod() {
    return abonentStatusMethod;
  }

  public void setAbonentStatusMethod(String abonentStatusMethod) {
    this.abonentStatusMethod = abonentStatusMethod;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }
}
