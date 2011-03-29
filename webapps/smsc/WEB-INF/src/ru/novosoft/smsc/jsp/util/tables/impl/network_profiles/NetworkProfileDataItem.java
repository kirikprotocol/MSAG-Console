package ru.novosoft.smsc.jsp.util.tables.impl.network_profiles;

import ru.novosoft.smsc.admin.network_profiles.NetworkProfile;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

/**
 * @author Aleksandr Khalitov
 */
public class NetworkProfileDataItem extends AbstractDataItem {

  NetworkProfileDataItem(NetworkProfile profile) {
    values.put("name", profile.getName());
    values.put("abonentStatusMethod", profile.getAbonentStatusMethod());
    values.put("ussdOpenDestRef", profile.getUssdOpenDestRef());
    values.put("vlrDetectionMethod", profile.getVlrDetectionMethod());
  }

  public String getName() {
    return (String)getValue("name");
  }

  public String getUssdOpenDestRef() {
    return (String)getValue("ussdOpenDestRef");
  }

  public String getAbonentStatusMethod() {
    return (String)getValue("abonentStatusMethod");
  }

  public String getVlrDetectionMethod() {
    return (String)getValue("vlrDetectionMethod");
  }

}