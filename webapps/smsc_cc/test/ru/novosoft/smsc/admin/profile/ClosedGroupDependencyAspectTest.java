package ru.novosoft.smsc.admin.profile;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;

import static org.junit.Assert.assertFalse;

/**
 * @author Artem Snopkov
 */
public class ClosedGroupDependencyAspectTest {

  private static File baseDir;
  private static AdminContext ctx;

  @BeforeClass
  public static void before() throws Exception {
    try {
      baseDir = TestUtils.createRandomDir("ctx");
      ctx = new TestAdminContext(baseDir, new File("test", "webconfig.xml"));
    } catch (Exception e) {
      e.printStackTrace();
      throw e;
    }
  }

  @AfterClass
  public static void after() throws AdminException {
    ctx.shutdown();
    if (baseDir != null)
      TestUtils.recursiveDeleteFolder(baseDir);
  }

  @Test
  public void testBeforeUpdateProfile() throws Exception {
    ProfileManager pm = ctx.getProfileManager();
    Profile p = new Profile();
    p.setAddress(new Address(".5.0.Test"));
    p.setLocale("en_en");
    p.setGroupId(87987978); // Invalid group

    try {
      pm.updateProfile(p);
      assertFalse(true);
    } catch (ProfileException e) {
      System.out.println(e.getMessage());
    }
  }
}
