package ru.novosoft.smsc.infosme.backend.config.retrypolicies;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataItem;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.ErrorRetryPolicy;
import ru.novosoft.smsc.admin.AdminException;

import java.util.*;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class RetryPolicy {

  private String id;
  private Map policies = new HashMap();
  private int defaultPolicy;
  private boolean modified;

  public RetryPolicy() {
    modified = true;
  }

  RetryPolicy(String id, Config config) throws AdminException {
    String sec = RetryPolicyManager.RETRY_POLICIES_SECTION + '.' + id;
    if (!config.containsSection(sec))
      throw new AdminException("Retry Policy " + id + " does not exists");

    try {
      this.id = id;
      this.defaultPolicy = config.getInt(sec + ".default");
      readErrorPolicies(config, id);

      modified = false;
    } catch (Throwable e) {
      throw new AdminException(e.getMessage());
    }
  }

  private void readErrorPolicies(Config config, String id) throws Config.WrongParamTypeException, Config.ParamNotFoundException {
    Collection errorPolicies = config.getSectionChildShortParamsNames(id);
    for (Iterator iter1 = errorPolicies.iterator(); iter1.hasNext();) {
      String errors = (String)iter1.next();
      if (errors.equals("default"))
        continue;
      int policy = config.getInt(id + '.' + errors);
      StringTokenizer st = new StringTokenizer(errors, ",");
      while (st.hasMoreTokens()) {
        String token = st.nextToken();
        policies.put(token, new Integer(policy));
      }
    }
  }

  void storeToConfig(Config config) {
    config.removeSection(RetryPolicyManager.RETRY_POLICIES_SECTION + '.' + id);
    String secName = RetryPolicyManager.RETRY_POLICIES_SECTION + '.' + id + '.';
    config.setInt(secName + "default", defaultPolicy);
    for (Iterator iter = policies.entrySet().iterator(); iter.hasNext();) {
      Map.Entry e = (Map.Entry)iter.next();
      config.setInt(secName + e.getKey(), ((Integer)e.getValue()).intValue());
    }    
  }

  public int getDefaultPolicy() {
    return defaultPolicy;
  }

  public void setDefaultPolicy(int defaultPolicy) {
    this.defaultPolicy = defaultPolicy;
    setModified(true);
  }

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
    setModified(true);
  }

  public Map getPolicies() {
    return policies;
  }

  public void setPolicies(Map policies) {
    this.policies = policies;
    setModified(true);
  }

  public boolean isModified() {
    return modified;
  }

  public void setModified(boolean modified) {
    this.modified = modified;
  }
}
