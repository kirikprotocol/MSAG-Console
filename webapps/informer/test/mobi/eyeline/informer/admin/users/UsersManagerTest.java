package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.junit.AfterClass;
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

  private static UsersManager usersManager;

  @BeforeClass
  public static void init() throws Exception {
    configFile = TestUtils.exportResourceToRandomFile(UsersManagerTest.class.getResourceAsStream("users.xml"), ".user");
    backupDir = TestUtils.createRandomDir(".users.backup");
    usersManager = new UsersManager(configFile, backupDir, FileSystem.getFSForSingleInst());
  }

  @Test
  public void loadSave() throws Exception{
    UsersSettings us = usersManager.getUsersSettings();
    Collection<User> old = us.getUsers();
    UsersSettings newUs = new UsersSettings();
    Collection<User> newU = new ArrayList<User>(old.size());
    for(User u : old) {
      newU.add(new User(u));
    }
    newUs.setUsers(newU);
    usersManager.updateSettings(newUs);

    newUs = usersManager.getUsersSettings();
    newU = newUs.getUsers();

    assertTrue(newU.size() == old.size());

    for(User o : old) {
      boolean found = false;
      for(User n : newU) {
        if(o.getLogin().equals(n.getLogin())) {
          assertEquals(o.getPhone(), n.getPhone());
          assertEquals(o.getEmail(), n.getEmail());
          assertEquals(o.getFirstName(), n.getFirstName());
          assertEquals(o.getLastName(), n.getLastName());
          assertEquals(o.getStatus(), n.getStatus());
          assertEquals(o.getPassword(), n.getPassword());
          assertEquals(o.getOrganization(), n.getOrganization());
          assertEquals(o.getLocale(), n.getLocale());
          assertEquals(o.getRoles().size(), n.getRoles().size());
          for(String ro : o.getRoles()) {
            boolean foundR = false;
            for(String no : n.getRoles()) {
              if(no.equals(ro)) {
                foundR = true;
                break;
              }
            }
            assertTrue(foundR);
          }

          found = true;
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
