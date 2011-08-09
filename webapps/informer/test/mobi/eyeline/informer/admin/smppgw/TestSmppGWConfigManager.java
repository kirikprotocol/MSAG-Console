package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.InitException;

import java.io.File;

/**
 * author: Aleksandr Khalitov
 */
public class TestSmppGWConfigManager extends SmppGWConfigManager{

  public TestSmppGWConfigManager(File config, File backup, SmppGWConfigManagerContext context) throws InitException {
    super(new TestSmppGW(), config, backup, context);
  }
}
