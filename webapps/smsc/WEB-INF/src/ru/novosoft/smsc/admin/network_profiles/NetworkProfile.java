package ru.novosoft.smsc.admin.network_profiles;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;

/**
 * @author Aleksandr Khalitov
 */
public class NetworkProfile {

  private Map masks = new TreeMap();

  private String ussdOpenDestRef;

  private String abonentStatusMethod;

  private String name;

  public NetworkProfile(NetworkProfile networkProfile) throws AdminException {
    this.ussdOpenDestRef = networkProfile.ussdOpenDestRef;
    this.abonentStatusMethod = networkProfile.abonentStatusMethod;
    this.name = networkProfile.name;
    Iterator mI = networkProfile.masks.values().iterator();
    while(mI.hasNext()) {
      Mask m = (Mask)mI.next();
      this.masks.put(m.getMaskSimple(), new Mask(m.getMaskSimple()));
    }
  }

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
