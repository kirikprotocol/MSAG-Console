package ru.novosoft.smsc.infosme.backend.config.retrypolicies;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class RetryPolicyManager {

  public static final String RETRY_POLICIES_SECTION = "InfoSme.RetryPolicies";

  private final Map policies = new HashMap();
  private boolean modified;

  public RetryPolicyManager(Config cfg) throws AdminException {
    resetRetryPolicies(cfg);
  }

  public synchronized void addRetryPolicy(RetryPolicy p) {
    policies.put(p.getId(), p);
  }

  public synchronized void removeRetryPolicy(String id) {
    policies.remove(id);
    modified = true;
  }

  public synchronized RetryPolicy getRetryPolicy(String id) {
    return (RetryPolicy)policies.get(id);
  }

  public synchronized List getRetryPolicies() {
    return new ArrayList(policies.values());
  }

  public synchronized boolean containsRetryPolicy(String id) {
    return policies.containsKey(id);
  }

  public synchronized boolean isRetryPoliciesChanged() {
    if (modified)
      return true;
    for (Iterator iter = policies.values().iterator(); iter.hasNext();) {
      RetryPolicy p = (RetryPolicy)iter.next();
      if (p.isModified())
        return true;
    }
    return false;
  }

  public synchronized void applyRetryPolicies(Config cfg) throws AdminException {
    try {
      // Remove old policies
      cfg.removeSection(RETRY_POLICIES_SECTION);

      // Add new policies
      for (Iterator iter = policies.values().iterator(); iter.hasNext();) {
        RetryPolicy p = (RetryPolicy)iter.next();
        p.storeToConfig(cfg);
      }

    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public synchronized void resetRetryPolicies(Config cfg) throws AdminException {
    policies.clear();
    try {
      for (Iterator iter = loadPolicies(cfg).iterator(); iter.hasNext();) {
        RetryPolicy p = (RetryPolicy)iter.next();
        policies.put(p.getId(), p);
      }
      modified = false;
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void setModified(boolean modified) {
    this.modified = modified;
    for (Iterator iter = policies.values().iterator(); iter.hasNext();) {
        RetryPolicy p = (RetryPolicy)iter.next();
        p.setModified(false);
      }
  }

  private List loadPolicies(Config cfg) throws AdminException {
    if (!cfg.containsSection(RETRY_POLICIES_SECTION))
      return Collections.EMPTY_LIST;

    List result = new ArrayList(100);
    try {
      Set policies = cfg.getSectionChildShortSectionNames(RETRY_POLICIES_SECTION);
      for (Iterator iter = policies.iterator(); iter.hasNext();)
        result.add(new RetryPolicy((String)iter.next(), cfg));
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
    return result;
  }

}
