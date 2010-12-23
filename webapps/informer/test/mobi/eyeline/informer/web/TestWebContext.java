package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.TestAdminContext;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.util.xml.WebXml;
import mobi.eyeline.informer.web.config.TimezonesConfig;
import testutils.TestUtils;

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
    TestUtils.exportResource(TimezonesConfig.class.getResourceAsStream("timezones.xml"), new File(confDir, "timezones.xml"), false);
  }

  public TestWebContext(WebXml webXml, File baseDir) throws InitException {
    try {
      prepare(baseDir);
    } catch (IOException e) {
      throw new InitException(e);
    }
    init(webXml, baseDir, new TestAdminContext(baseDir));
  }
}
