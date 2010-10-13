package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.BaseManager;
import mobi.eyeline.informer.admin.util.config.SettingsReader;
import mobi.eyeline.informer.admin.util.config.SettingsWriter;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 17:41:06
 */
public class RetryPolicyManager extends BaseManager<RetryPolicySettings> {

  public RetryPolicyManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme,  config, backup, fileSystem,new RetryPolicyConfig());
  }

  public void addRetryPolicy(final RetryPolicy rp) throws AdminException{
    updateSettings(new SettingsWriter<RetryPolicySettings>() {
      public void changeSettings(RetryPolicySettings settings) throws AdminException {
        settings.addRetryPolicy(rp);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.addRetryPolicy(rp.getPolicyId());
      }
    });
  }

  public void updateRetryPolicy(final RetryPolicy rp) throws AdminException{
    updateSettings(new SettingsWriter<RetryPolicySettings>() {
      public void changeSettings(RetryPolicySettings settings) throws AdminException{
        settings.updateRetryPolicy(rp);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.updateRetryPolicy(rp.getPolicyId());
      }
    });
  }

  public void removeRetryPolicy(final String policyId) throws AdminException{
    updateSettings(new SettingsWriter<RetryPolicySettings>() {
      public void changeSettings(RetryPolicySettings settings) throws AdminException {
        settings.removeRetryPolicy(policyId);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.removeRetryPolicy(policyId);
      }
    });
  }

  public List<RetryPolicy> getRetryPolicies()  {
    return readSettings(new SettingsReader<RetryPolicySettings, List<RetryPolicy>>(){
      public List<RetryPolicy> executeRead(RetryPolicySettings settings)  {
        List<RetryPolicy> result = new ArrayList<RetryPolicy>();
        for(RetryPolicy rp : settings.getPolicies()) {
          result.add(new RetryPolicy(rp));
        }
        return result;
      }
    });

  }

  public RetryPolicy getRetryPolicy(final String policyId)  {
    return readSettings(new SettingsReader<RetryPolicySettings, RetryPolicy>(){
      public RetryPolicy executeRead(RetryPolicySettings settings)  {
        RetryPolicy rp = settings.getRetryPolicy(policyId);
        return rp == null ? null : new RetryPolicy(rp);
      }
    });
  }

}
