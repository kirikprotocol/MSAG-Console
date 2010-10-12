package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;

import java.io.File;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 18:27:47
 */
public class TestRetryPolicyManager extends RetryPolicyManager {
  public TestRetryPolicyManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme, config, backup, fileSystem);
  }
  
}
