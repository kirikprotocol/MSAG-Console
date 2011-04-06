package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.TestAdminContext;

import java.io.File;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class TestWebContext extends WebContext {

  private static void prepare(File baseDir) throws IOException {
    File confDir = new File(baseDir, "conf");
    if (!confDir.exists())
      confDir.mkdirs();
    }

  public TestWebContext(File baseDir) throws InitException {
    try {
      prepare(baseDir);
    } catch (IOException e) {
      throw new InitException(e);
    }
    init(baseDir, new TestAdminContext(baseDir));
  }
}
