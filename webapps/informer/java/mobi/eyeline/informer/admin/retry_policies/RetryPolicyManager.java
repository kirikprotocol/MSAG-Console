package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 17:41:06
 */
public class RetryPolicyManager extends BaseManager<RetryPolicySettings>{


  public RetryPolicyManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme,  config, backup, fileSystem,new RetryPolicyConfig());
  }

  public void addRetryPolicy(final RetryPolicy rp) throws AdminException{
    new BaseManagerWriteExecutor() {
      @Override
      void changeSettings(RetryPolicySettings settings) throws AdminException {
        settings.addRetryPolicy(rp);
      }
      @Override
      void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.addRetryPolicy(rp.getPolicyId());
      }
    }.execute();
  }

  public void updateRetryPolicy(final RetryPolicy rp) throws AdminException{
    new BaseManagerWriteExecutor() {
      @Override
      void changeSettings(RetryPolicySettings settings) throws AdminException{
        settings.updateRetryPolicy(rp);
      }
      @Override
      void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.updateRetryPolicy(rp.getPolicyId());
      }
    }.execute();
  }

  public void removeRetryPolicy(final String policyId) throws AdminException{
    new BaseManagerWriteExecutor() {
      @Override
      void changeSettings(RetryPolicySettings settings) throws AdminException {
        settings.removeRetryPolicy(policyId);
      }
      @Override
      void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.removeRetryPolicy(policyId);
      }
    }.execute();
  }

  public List<RetryPolicy> getRetryPolicies() throws AdminException {
    return (new BaseManagerReadExecutor<List<RetryPolicy>>(){
      @Override
      List<RetryPolicy> executeRead(RetryPolicySettings settings) throws AdminException {
        List<RetryPolicy> result = new ArrayList<RetryPolicy>();
        for(RetryPolicy rp : settings.getPolicies()) {
          result.add(new RetryPolicy(rp));
        }
        return result;
      }
    }).execute();

  }

  public RetryPolicy getRetryPolicy(final String policyId) throws AdminException {
    return (new BaseManagerReadExecutor<RetryPolicy>(){
      @Override
      RetryPolicy executeRead(RetryPolicySettings settings) throws AdminException {
        RetryPolicy rp = settings.getRetryPolicy(policyId);
        return rp == null ? null : new RetryPolicy(rp);
      }
    }).execute();
  }

}
