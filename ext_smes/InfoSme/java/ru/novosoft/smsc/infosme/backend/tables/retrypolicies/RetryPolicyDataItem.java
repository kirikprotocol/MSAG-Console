package ru.novosoft.smsc.infosme.backend.tables.retrypolicies;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.List;

/**
 * User: artem
 * Date: 01.11.2008
 */
public class RetryPolicyDataItem extends AbstractDataItem {

  public RetryPolicyDataItem(String name, int defaultPolicy, List errorRetryPolicies) {
    values.put("name", name);
    values.put("errors", errorRetryPolicies);
    values.put("default", new Integer(defaultPolicy));
  }

  public String getName() {
    return (String)values.get("name");
  }

  public List getErrorsPolicies() {
    return (List)values.get("errors");
  }

  public int getDefaultPolicy() {
    return ((Integer)values.get("default")).intValue();
  }

}
