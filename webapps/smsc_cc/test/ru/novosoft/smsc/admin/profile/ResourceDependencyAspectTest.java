package ru.novosoft.smsc.admin.profile;

import org.junit.*;

import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.admin.filesystem.MemoryFileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class ResourceDependencyAspectTest {

  private AdminContext ctx;

  @Before
  public void before() throws Exception {
    try {
      MemoryFileSystem fs = new MemoryFileSystem();
      ctx = new TestAdminContext(fs.mkdirs("ctx"), new File("test", "webconfig.xml"), 2, fs);
    } catch (Exception e) {
      e.printStackTrace();
      throw e;
    }
  }

  @After
  public  void after() throws AdminException {
    ctx.shutdown();
  }

  @Test
  public void testBeforeUpdateProfile() throws Exception {
    ProfileManager pm = ctx.getProfileManager();
    Profile p = new Profile();
    p.setAddress(new Address(".5.0.Test"));
    p.setLocale("unknown locale");

    try {
      pm.updateProfile(p);
      assertFalse(true);
    } catch (ProfileException e) {
      System.out.println(e.getMessage());
    }
  }
}
