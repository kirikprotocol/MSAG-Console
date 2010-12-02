package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestUserStatProvider extends UserStatProvider {
  public TestUserStatProvider(File baseDir, FileSystem fileSys) {
    super(baseDir, fileSys, "'p'yyyyMMdd");
  }
}
