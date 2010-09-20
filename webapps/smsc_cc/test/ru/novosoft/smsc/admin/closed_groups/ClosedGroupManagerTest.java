package ru.novosoft.smsc.admin.closed_groups;


import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.filesystem.TestFileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class ClosedGroupManagerTest {

  private static File configFile, backupDir;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(ClosedGroupManagerTest.class.getResourceAsStream("ClosedGroups.xml"), ".closedgroups");
    backupDir = TestUtils.createRandomDir(".closedgroupsbackup");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  private void validate(ClosedGroupManager cgm) throws AdminException {
    assertEquals(2, cgm.getLastGroupId());

    ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
    assertEquals(2, groups.size());

    ClosedGroup g1 = groups.get(0);
    assertEquals("2", g1.getName());
    assertEquals("234", g1.getDescription());
    assertEquals(2, g1.getId());

    ClosedGroup g2 = groups.get(1);
    assertEquals("qq", g2.getName());
    assertEquals("qqqq", g2.getDescription());
    assertEquals(1, g2.getId());
    ArrayList<Address> g2a = new ArrayList<Address>(g2.getMasks());
    assertEquals(2, g2a.size());
    assertEquals(new Address(".1.1.79495445566"), g2a.get(0));
    assertEquals(new Address(".1.1.79139495113"), g2a.get(1));
  }

  @Test
  public void loadTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    validate(cgm);
  }

  @Test
  public void saveTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    cgm.save();

    ClosedGroupManager cgm1 = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    validate(cgm1);
  }

  @Test
  public void addGroupTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    ClosedGroup cg = cgm.addGroup("newgroup", "newdescription");

    assertEquals(cgm.getLastGroupId(), 3);
    assertEquals("newgroup", cg.getName());
    assertEquals("newdescription", cg.getDescription());
    assertEquals(0, cg.getMasks().size());


    ClosedGroupManager cgm1 = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
    assertEquals(cgm1.getLastGroupId(), 3);
    assertEquals(3, groups.size());

    ClosedGroup cg1 = groups.get(2);

    assertEquals("newgroup", cg1.getName());
    assertEquals("newdescription", cg1.getDescription());
  }

  @Test
  public void removeGroupTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    assertNull(cgm.removeGroup(100));

    assertNotNull(cgm.removeGroup(1) );
    assertEquals(1, cgm.groups().size());

    ClosedGroupManager cgm1 = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    assertEquals(1, cgm1.groups().size());
    assertEquals(2, new ArrayList<ClosedGroup>(cgm1.groups()).get(0).getId());
  }

  @Test
  public void addMaskTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    ClosedGroup newGroup = cgm.addGroup("newgroup", "newdescription");
    newGroup.addMask(new Address(".1.1.79134565334"));

    ClosedGroup group2 = new ArrayList<ClosedGroup>(cgm.groups()).get(0);
    group2.addMask(new Address(".1.1.79134565334"));

    ClosedGroup groupQq = new ArrayList<ClosedGroup>(cgm.groups()).get(1);
    groupQq.addMask(new Address(".1.1.79134565334"));

    ClosedGroupManager cgm1 = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
    List<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm1.groups());

    assertTrue(groups.get(0).getMasks().contains(new Address(".1.1.79134565334")));
    assertTrue(groups.get(1).getMasks().contains(new Address(".1.1.79134565334")));
    assertTrue(groups.get(2).getMasks().contains(new Address(".1.1.79134565334")));
  }

  @Test
  public void removeMaskTest() throws AdminException {
    {
      ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
      ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
      ClosedGroup groupQQ = groups.get(1);
      assertTrue(groupQQ.getMasks().contains(new Address("+79495445566")));
      groupQQ.removeMask(new Address("+79495445566"));
    }

    {
      ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
      ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
      ClosedGroup groupQQ = groups.get(1);
      assertFalse(groupQQ.getMasks().contains(new Address("+79495445566")));
    }
  }

  @Test
  public void addGroupErrorTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new ErrorClusterController(), FileSystem.getFSForSingleInst());
    assertEquals(2, cgm.groups().size());
    try {
      cgm.addGroup("newGroup", "newGroup");
      assertTrue(false);
    } catch (AdminException e) {
    }
    assertEquals(2, cgm.groups().size());
  }

  @Test
  public void removeGroupErrorTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new ErrorClusterController(), FileSystem.getFSForSingleInst());
    assertEquals(2, cgm.groups().size());
    try {
      cgm.removeGroup(2);
      assertTrue(false);
    } catch (AdminException e) {
    }
    assertEquals(2, cgm.groups().size());
  }

  @Test
  public void addMaskErrorTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new ErrorClusterController(), FileSystem.getFSForSingleInst());
    ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
    ClosedGroup groupQQ = groups.get(1);

    try {
      groupQQ.addMask(new Address("+79495445546"));
      assertTrue(false);
    } catch (AdminException e) {
    }
    assertFalse(groupQQ.getMasks().contains(new Address("+79495445546")));
  }

  @Test
  public void removeMaskErrorTest() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new ErrorClusterController(), FileSystem.getFSForSingleInst());
    ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
    ClosedGroup groupQQ = groups.get(1);

    try {
      groupQQ.removeMask(new Address("+79495445566"));
      assertTrue(false);
    } catch (AdminException e) {
    }
    assertTrue(groupQQ.getMasks().contains(new Address("+79495445566")));
  }

  @Test
  public void setDescriptionTest() throws AdminException {
    {
      ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
      ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
      ClosedGroup groupQQ = groups.get(1);
      groupQQ.setDescription("newdescription");
    }

    {
      ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), FileSystem.getFSForSingleInst());
      ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
      ClosedGroup groupQQ = groups.get(1);
      assertEquals("newdescription", groupQQ.getDescription());
    }
  }

  @Test
  public void configBrokenTest() throws AdminException {

    ErrorFileSystem fs = new ErrorFileSystem();
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new TestClusterControllerStub(), fs);
    ArrayList<ClosedGroup> groups = new ArrayList<ClosedGroup>(cgm.groups());
    ClosedGroup groupQQ = groups.get(1);

    assertFalse(cgm.isBroken());

    try {
      cgm.removeGroup(1);
      assertFalse(true);
    } catch (AdminException e) {
    }

    assertTrue(cgm.isBroken());

    fs.error = false;

    try {
      cgm.addGroup("adsfasdf", "sdfasdf");
      assertFalse(true);
    } catch (AdminException e) {
    }

    try {
      cgm.groups();
      assertFalse(true);
    } catch (AdminException e) {
    }

    try {
      groupQQ.addMask(new Address("+63637823632"));
      assertFalse(true);
    } catch (AdminException e) {
    }

    try {
      groupQQ.removeMask(new Address("+63637823632"));
      assertFalse(true);
    } catch (AdminException e) {
    }

    try {
      groupQQ.setDescription("desc");
      assertFalse(true);
    } catch (AdminException e) {
    }
  }

  @Test
  public void testGetStatusForSmscs() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new ClusterControllerImpl(), FileSystem.getFSForSingleInst());

    Map<Integer, SmscConfigurationStatus> states = cgm.getStatusForSmscs();

    assertNotNull(states);
    assertEquals(2, states.size());
    assertEquals(SmscConfigurationStatus.OUT_OF_DATE, states.get(0));
    assertEquals(SmscConfigurationStatus.UP_TO_DATE, states.get(1));
  }

  @Test
  public void nullGetStatusForSmscs() throws AdminException {
    ClosedGroupManager cgm = new ClosedGroupManager(configFile, backupDir, new ClusterControllerImpl1(), FileSystem.getFSForSingleInst());

    Map<Integer, SmscConfigurationStatus> states = cgm.getStatusForSmscs();

    assertTrue(states.isEmpty());
  }

  public class ClusterControllerImpl extends TestClusterControllerStub {
    public ConfigState getClosedGroupConfigState() throws AdminException {
      long now = System.currentTimeMillis();
      Map<Integer, Long> map = new HashMap<Integer, Long>();
      map.put(0, now - 1);
      map.put(1, now);
      return new ConfigState(now, map);
    }
  }

  public class ClusterControllerImpl1 extends TestClusterControllerStub {
    public ConfigState getClosedGroupConfigState() throws AdminException {
      return null;
    }
  }


  private static class ErrorClusterController extends TestClusterControllerStub {
    public boolean isOnline() {
      return true;
    }

    public void addClosedGroup(int groupId, String groupName) throws AdminException {
      throw new ClosedGroupException("config_broken");
    }

    public void removeClosedGroup(int groupId) throws AdminException {
      throw new ClosedGroupException("config_broken");
    }

    public void addMaskToClosedGroup(int groupId, Address masks) throws AdminException {
      throw new ClosedGroupException("config_broken");
    }

    public void removeMaskFromClosedGroup(int groupId, Address masks) throws AdminException {
      throw new ClosedGroupException("config_broken");
    }
  }

  private static class ErrorFileSystem extends TestFileSystem {

    private boolean error = true;

    @Override
    public OutputStream getOutputStream(File file) throws AdminException {
      if (error)
        throw new ClosedGroupException("config_broken");
      else
        return FileSystem.getFSForSingleInst().getOutputStream(file);
    }
  }

}
