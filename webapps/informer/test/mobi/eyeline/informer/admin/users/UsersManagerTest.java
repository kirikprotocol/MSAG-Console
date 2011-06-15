package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.util.Collection;
import java.util.List;

import static org.junit.Assert.*;

/**
 * author: alkhal
 */
public class UsersManagerTest {

  private File configFile, backupDir;

  private Infosme infosme;
  private UsersManager usersManager;

  @Before
  public void before() throws Exception {
    configFile = TestUtils.exportResourceToRandomFile(UsersManagerTest.class.getResourceAsStream("users.xml"), ".user");
    backupDir = TestUtils.createRandomDir(".users.backup");
    infosme = new TestInfosme();
    usersManager = new TestUsersManager(infosme, configFile, backupDir, FileSystem.getFSForSingleInst());
    for(User u : usersManager.getUsers()) {
      infosme.addUser(u.getLogin());
    }
  }

  @Test
  public void loadSave() throws Exception{
    Collection<User> old = usersManager.getUsers();
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

  @Test
  public void testRemovingCPR() throws Exception {
    List<User> users = usersManager.getUsers();
    if(users.isEmpty()) {
      fail("Users are empty. Nothing to test");
    }
    User u = users.get(0);
    List<UserCPsettings> cpSettings = u.getCpSettings();
    if(cpSettings.isEmpty()) {
      fail("Settings are empty. Nothing to test");
    }
    UserCPsettings cps = cpSettings.remove(0);

    u.setCpSettings(cpSettings);
    usersManager.updateUser(u);

    usersManager = new TestUsersManager(infosme, configFile, backupDir, FileSystem.getFSForSingleInst());

    List<UserCPsettings> cpSettings1 = usersManager.getUser(u.getLogin()).getCpSettings();

    assertEquals(cpSettings.size(), cpSettings1.size());

    for(UserCPsettings cps1 : cpSettings1) {
      if(cps1.getHashId().equals(cps.getHashId())) {
       fail("Settings is not removed");
      }
    }

  }



  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  @After
  public void shutdown() {
    if(configFile != null) {
      configFile.delete();
    }
    if(backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }

  }
}
