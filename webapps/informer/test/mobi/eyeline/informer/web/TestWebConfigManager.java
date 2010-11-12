package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.WebConfigManager;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 14:29:40
 */
public class TestWebConfigManager extends WebConfigManager {
  public TestWebConfigManager(File config, File backup, FileSystem fileSys) throws InitException {
    super(config, backup, fileSys);
  }
}
