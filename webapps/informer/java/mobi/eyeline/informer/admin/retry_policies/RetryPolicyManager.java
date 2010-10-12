package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.ConfigFileManager;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 17:41:06
 */
public class RetryPolicyManager {
  private Infosme infosme;

  private final ConfigFileManager<RetryPolicySettings> cfgFileManager;

  private ReadWriteLock lock = new ReentrantReadWriteLock();

  private RetryPolicySettings settings;



  public RetryPolicyManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {

    this.infosme = infosme;

    cfgFileManager = new ConfigFileManager<RetryPolicySettings>(config, backup, fileSystem, new RetryPolicyConfig());
    try{
      this.settings = cfgFileManager.load();
    }catch (AdminException e){
      throw new InitException(e);
    }
  }


  private void save() throws AdminException {
      cfgFileManager.save(settings);
  }

  public void addRetryPolicy(RetryPolicy rp) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.addRetryPolicy(rp);
      save();
      if(infosme.isOnline()) {
        infosme.addRetryPolicy(rp.getPolicyId());
      }
    }finally {
      lock.writeLock().unlock();
    }

  }

  public void updateRetryPolicy(RetryPolicy rp) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.updateRetryPolicy(rp);
      save();
      if(infosme.isOnline()) {
        infosme.updateRetryPolicy(rp.getPolicyId());
      }
    }finally {
      lock.writeLock().unlock();
    }
  }

  public List<RetryPolicy> getRetryPolicies() {
    try{
      lock.readLock().lock();
      List<RetryPolicy> result = new ArrayList<RetryPolicy>();
      for(RetryPolicy rp : settings.getPolicies()) {
        result.add(new RetryPolicy(rp));
      }
      return result;
    }finally {
      lock.readLock().unlock();
    }
  }

  public RetryPolicy getRetryPolicy(String policyId){
    try{
      lock.readLock().lock();
      RetryPolicy rp = settings.getRetryPolicy(policyId);
      return rp == null ? null : new RetryPolicy(rp);
    }finally {
      lock.readLock().unlock();
    }
  }

  public void removeRetryPolicy(String policyId) throws AdminException{
    try{
      lock.writeLock().lock();
      settings.removeRetryPolicy(policyId);
      save();
      if(infosme.isOnline()) {
        infosme.removeRetryPolicy(policyId);
      }
    }finally {
      lock.writeLock().unlock();
    }
  }



}
