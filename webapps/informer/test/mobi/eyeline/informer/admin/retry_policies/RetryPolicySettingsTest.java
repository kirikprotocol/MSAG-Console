package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 18:05:44
 */
public class RetryPolicySettingsTest {


  RetryPolicy aPolicy;

  @Before
  public void before() throws AdminException {
    aPolicy = new RetryPolicy("aPolicy",2000);
  }


  @Test
  public void add() throws AdminException {
    RetryPolicySettings settings = new RetryPolicySettings();
    settings.addRetryPolicy(aPolicy);

    assertTrue(aPolicy == settings.getRetryPolicy(aPolicy.getPolicyId()));    
    assertTrue(settings.getPolicies().size() == 1);
    assertTrue(settings.getPolicies().iterator().next() == aPolicy);

    try{
      settings.addRetryPolicy(aPolicy);
      assertTrue(false);
    }catch (AdminException e){}
  }

  @Test
  public void update() throws AdminException {
    RetryPolicySettings settings = new RetryPolicySettings();

    settings.addRetryPolicy(aPolicy);

    aPolicy.setDefaultTime(3000);
    settings.updateRetryPolicy(aPolicy);

    assertTrue(aPolicy.equals(settings.getRetryPolicy(aPolicy.getPolicyId())));

    try{
      settings.updateRetryPolicy(new RetryPolicy("bPolicy",200));
      assertTrue(false);
    }
    catch (AdminException e){}

  }

  @Test
  public void testRemove() throws AdminException {
     RetryPolicySettings settings = new RetryPolicySettings();

     settings.addRetryPolicy(aPolicy);

     settings.removeRetryPolicy(aPolicy.getPolicyId());
     assertTrue(settings.getRetryPolicy(aPolicy.getPolicyId())==null);
     assertTrue(settings.getPolicies().size()==0);

     try{
       settings.updateRetryPolicy(aPolicy);
       assertTrue(false);
     }
     catch (AdminException e){}
  }

}
