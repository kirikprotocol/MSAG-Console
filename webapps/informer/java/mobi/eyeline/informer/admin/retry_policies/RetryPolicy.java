package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class RetryPolicy {

  private String policyId;
  private int defaultTime;
  private Map<Integer,RetryPolicyEntry> entryMap = new LinkedHashMap<Integer,RetryPolicyEntry>();
  private ValidationHelper vh = new ValidationHelper(RetryPolicySettings.class);


  public RetryPolicy(String policyId,int defaultTime) throws AdminException {
    vh.checkPositive("time",defaultTime);
    vh.checkNotEmpty("name",policyId);
    this.policyId = policyId;
    this.defaultTime = defaultTime;
  }

  public RetryPolicy(RetryPolicy copy) {
    this.policyId = copy.policyId;
    this.defaultTime = copy.defaultTime;
    this.entryMap = new LinkedHashMap<Integer,RetryPolicyEntry>();
    for(RetryPolicyEntry entry : copy.getEntries()) {
      this.entryMap.put(entry.getErrCode(), new RetryPolicyEntry(entry));
    }
  }


  public String getPolicyId() {
    return policyId;
  }

  

  public int getDefaultTime() {
    return defaultTime;
  }

  public void setDefaultTime(int defaultTime) throws AdminException {
    vh.checkPositive("time",defaultTime);
    this.defaultTime = defaultTime;
  }

  public List<RetryPolicyEntry> getEntries() {
    return new ArrayList<RetryPolicyEntry>(entryMap.values());
  }

  public void setEntries(List<RetryPolicyEntry> entries) throws AdminException {
    vh.checkNotNull("policies",entries);
    LinkedHashMap<Integer, RetryPolicyEntry> newEntryMap = new LinkedHashMap<Integer,RetryPolicyEntry>();
    for(RetryPolicyEntry entry : entries) {
      vh.checkNotСontainsKey("errorCode",newEntryMap,entry.getErrCode());
      newEntryMap.put(entry.getErrCode(),entry);
    }
    this.entryMap = newEntryMap;
  }

  public Map<Integer, RetryPolicyEntry> getEntryMap() {
    return entryMap;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    RetryPolicy that = (RetryPolicy) o;

    if (defaultTime != that.defaultTime) return false;
    if (entryMap != null ? !entryMap.equals(that.entryMap) : that.entryMap != null) return false;
    if (policyId != null ? !policyId.equals(that.policyId) : that.policyId != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = policyId != null ? policyId.hashCode() : 0;
    result = 31 * result + defaultTime;
    result = 31 * result + (entryMap != null ? entryMap.hashCode() : 0);
    return result;
  }

  @Override
  public String toString() {
    return "RetryPolicy{" +
        "name='" + policyId + '\'' +
        ", defaultTime=" + defaultTime +
        ", entries=" + entryMap +
        '}';
  }
}
