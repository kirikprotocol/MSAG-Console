package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 12:39:36
 */
class RetryPolicySettings {
  private Map<String, RetryPolicy> policies = new LinkedHashMap<String, RetryPolicy>();

  private ValidationHelper vh = new ValidationHelper(RetryPolicySettings.class);


  public RetryPolicySettings() {

  }

  RetryPolicySettings(RetryPolicySettings copy) {
    for(RetryPolicy p : copy.getPolicies()) {
      this.policies.put(p.getPolicyId(),new RetryPolicy(p));
    }
  }


  public List<RetryPolicy> getPolicies() {
    return new ArrayList<RetryPolicy>(policies.values());
  }



  public void addRetryPolicy(RetryPolicy p) throws AdminException{
    vh.checkNotСontainsKey("name",policies,p.getPolicyId());    
    policies.put(p.getPolicyId(),p);
  }

  public void updateRetryPolicy( RetryPolicy p) throws AdminException {
    vh.checkСontainsKey("name",policies,p.getPolicyId());
    policies.put(p.getPolicyId(),p);
  }

  public void removeRetryPolicy(String policyId) throws AdminException {
    policies.remove(policyId);
  }


  public RetryPolicy getRetryPolicy(String policyId) {
    return policies.get(policyId);
  }
}
