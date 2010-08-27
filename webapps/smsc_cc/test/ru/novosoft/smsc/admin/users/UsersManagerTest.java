package ru.novosoft.smsc.admin.users;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
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
          assertEquals(o.getCellPhone(), n.getCellPhone());
          assertEquals(o.getDept(), n.getDept());
          assertEquals(o.getEmail(), n.getEmail());
          assertEquals(o.getFirstName(), n.getFirstName());
          assertEquals(o.getHomePhone(), n.getHomePhone());
          assertEquals(o.getLastName(), n.getLastName());
          assertEquals(o.getPassword(), n.getPassword());
          assertEquals(o.getWorkPhone(), n.getWorkPhone());
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
          UserPreferences oPr = o.getPrefs();
          UserPreferences nPr = n.getPrefs();

          assertEquals(oPr.getLocale(),nPr.getLocale());
          assertEquals(oPr.getPerfMonBlock(),nPr.getPerfMonBlock());
          assertEquals(oPr.getPerfMonPixPerSecond(),nPr.getPerfMonPixPerSecond());
          assertEquals(oPr.getPerfMonScale(),nPr.getPerfMonScale());
          assertEquals(oPr.getPerfMonVLightGrid(),nPr.getPerfMonVLightGrid());
          assertEquals(oPr.getTopMonGraphGrid(),nPr.getTopMonGraphGrid());
          assertEquals(oPr.getPerfMonVMinuteGrid(),nPr.getPerfMonVMinuteGrid());
          assertEquals(oPr.getTopMonGraphHead(),nPr.getTopMonGraphHead());
          assertEquals(oPr.getTopMonGraphScale(),nPr.getTopMonGraphScale());
          assertEquals(oPr.getTopMonMaxSpeed(),nPr.getTopMonMaxSpeed());
          assertEquals(oPr.getTopMonMaxSpeed(),nPr.getTopMonMaxSpeed());
          assertEquals(oPr.getTopMonGraphHigrid(),nPr.getTopMonGraphHigrid());
          assertEquals(oPr.getUnknown().size(), nPr.getUnknown().size());

          for(Map.Entry<String, String> uo : oPr.getUnknown().entrySet()) {
            boolean foundR = false;
            for(Map.Entry<String, String> un : nPr.getUnknown().entrySet()) {
              if(uo.equals(un)) {
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
