package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceManager;

/**
 * author: Aleksandr Khalitov
 */
public class TestMCISmeManager extends MCISmeManagerImpl{

  public TestMCISmeManager(ServiceManager serviceManager, FileSystem fs) throws AdminException {
    super(serviceManager, fs);
  }
}
