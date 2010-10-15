package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.admin.regions.RegionsManager;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * author: alkhal
 */
public class UsersManagerTest {

  private static File configFile, backupDir;

  private static Infosme infosme;
  private static UsersManager usersManager;

  @BeforeClass
  public static void init() throws Exception {
    configFile = TestUtils.exportResourceToRandomFile(UsersManagerTest.class.getResourceAsStream("users.xml"), ".user");
    backupDir = TestUtils.createRandomDir(".users.backup");
  }

  @Before
  public void before() throws Exception {
    infosme = new TestInfosme();
    usersManager = new TestUsersManager(infosme, configFile, backupDir, FileSystem.getFSForSingleInst());
    for(User u : usersManager.getUsers()) {
      infosme.addUser(u.getLogin());
    }
  }

  @Test
  public void loadSave() throws Exception{
    Collection<User> old = usersManager.getUsers();
    Collection<User> newU = new ArrayList<User>(old.size());
    for(User u : usersManager.getUsers()) {
      usersManager.updateUser(new User(u));
    }

    assertTrue(usersManager.getUsers().size() == old.size());

    for(User o : old) {
      boolean found = false;
      for(User n : usersManager.getUsers()) {
        if(o.getLogin().equals(n.getLogin())) {
          UserTestUtils.compareUsers(o,n);
          found=true;
          break;
        }
      }
      assertTrue(found);
    }
  }



  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  @AfterClass
  public static void shutdown() {
    if(configFile != null) {
      configFile.delete();
    }
    if(backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }

  }
}
