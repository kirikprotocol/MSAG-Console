package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.util.Address;
import org.junit.*;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import testutils.TestUtils;

import java.io.File;
import java.util.TimeZone;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.Assert.*;
import static org.junit.Assert.assertFalse;

/**
 * @author Aleksandr Khalitov
 */

public class RegionsManagerTest {

  private static final AtomicInteger regNameSyffix = new AtomicInteger(0);

  protected File configFile, backupDir;

  @Before
  public void init() throws Exception {
    configFile = TestUtils.exportResourceToRandomFile(RegionsManager.class.getResourceAsStream("regions.xml"), ".regions");
    backupDir = TestUtils.createRandomDir(".config.backup");
  }

  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  @After
  public void shutdown() {
    if (configFile != null) {
      configFile.delete();
    }
    if (backupDir != null) {
      TestUtils.recursiveDeleteFolder(backupDir);
    }
  }

  protected Region getExistedRegion() throws AdminException, InitException {
    return createManager().getRegions().get(0);
  }

  protected Region getExistedRegion1() throws AdminException, InitException {
    return createManager().getRegions().get(1);
  }

  protected RegionsManager createManager() throws AdminException, InitException {
    return createManager(false);
  }

  protected RegionsManager createManager(boolean errorMode) throws AdminException, InitException {
    Infosme infosem = new TestInfosme(errorMode);
    return new RegionsManager(infosem, configFile, backupDir, FileSystem.getFSForSingleInst());
  }

  private static Region createValidRegion() {
    Region r = buildRegionWOMask();
    r.addMask(new Address("+791394899??"));
    r.addMask(new Address("+791394898??"));
    return r;
  }

  private static Region buildRegionWOMask() {
    Region r = new Region();
    r.setMaxSmsPerSecond(200);
    r.setName("MR SIBIR'" + regNameSyffix.incrementAndGet());
    r.setSmsc("SMSC2");
    r.setTimeZone(TimeZone.getDefault());
    return r;
  }

  private void assertAddRegionFailed(Region r) throws Exception {
    RegionsManager regionsManager = createManager();
    try {
      regionsManager.addRegion(r);
      assertTrue(false);
    } catch (AdminException e) {
    }
  }

  private void assertUpdateRegionFailed(Region r) throws Exception {
    RegionsManager regionsManager = createManager();
    try {
      regionsManager.updateRegion(r);
      assertTrue(false);
    } catch (AdminException e) {
    }
  }


  // ПРОВЕРКА ПАРАМЕТРОВ

  @Test
  public void testAddRegionWithInvalidName() throws Exception {
    Region r = createValidRegion();
    Region r1 = getExistedRegion();

    r.setName("");
    r1.setName("");
    assertAddRegionFailed(r);
    assertUpdateRegionFailed(r1);

    r.setName(null);
    r1.setName(null);
    assertAddRegionFailed(r);
    assertUpdateRegionFailed(r1);
  }

  @Test
  public void testAddRegionWithInvalidSmsc() throws Exception {
    Region r = createValidRegion();
    Region r1 = getExistedRegion();

    r.setSmsc("");
    r1.setSmsc("");
    assertAddRegionFailed(r);
    assertUpdateRegionFailed(r1);

    r.setSmsc(null);
    r1.setSmsc(null);
    assertAddRegionFailed(r);
    assertUpdateRegionFailed(r1);
  }

  @Test
  public void testAddRegionWithInvalidMaxSmsPerSecond() throws Exception {
    Region r = createValidRegion();
    Region r1 = getExistedRegion();

    r.setMaxSmsPerSecond(-1);
    r1.setMaxSmsPerSecond(-1);
    assertAddRegionFailed(r);
    assertUpdateRegionFailed(r1);
  }

  @Test
  public void testAddRegionWithZeroMaxSmsPerSecond() throws Exception {
    Region r = createValidRegion();
    Region r1 = getExistedRegion();

    r.setMaxSmsPerSecond(0);
    r1.setMaxSmsPerSecond(0);

    RegionsManager manager = createManager();
    manager.addRegion(r);
    manager.updateRegion(r1);
  }

  @Test
  public void testAddRegionWithInvalidTimezone() throws Exception {
    Region r = createValidRegion();
    Region r1 = getExistedRegion();

    r.setTimeZone(null);
    r1.setTimeZone(null);
    assertAddRegionFailed(r);
    assertUpdateRegionFailed(r1);
  }

  @Test
  public void testAddRegionWithSameName() throws Exception {
    Region r = createValidRegion();
    Region r1 = getExistedRegion();
    Region r2 = getExistedRegion1();

    r.setName(r1.getName());
    r2.setName(r1.getName());
    assertAddRegionFailed(r);
    assertUpdateRegionFailed(r2);
  }

  @Test
  public void testAddRegionWithSameMask() throws Exception {
    Region r = createValidRegion();
    Region r1 = getExistedRegion();
    Region r2 = getExistedRegion1();

    r.addMasks(r1.getMasks());
    r2.addMasks(r1.getMasks());
    assertAddRegionFailed(r);
    assertUpdateRegionFailed(r2);
  }

  @Test
  public void testSetInvalidDefaultMaxPerSecond() throws AdminException, InitException {
    RegionsManager manager = createManager();
    try {
      manager.setDefaultMaxPerSecond(-1);
      assertTrue(false);
    } catch (AdminException e) {
    }

    try {
      manager.setDefaultMaxPerSecond(0);
      assertTrue(false);
    } catch (AdminException e) {
    }
  }

  @Test
  public void testSetZeroValueToDefaultMaxPerSecond() throws Exception {
    RegionsManager manager = createManager();
    manager.setDefaultMaxPerSecond(0);
  }

  // ПРОВЕРКА ОПЕРАЦИЙ

  @Test
  public void testAddRegion() throws AdminException, InitException {
    RegionsManager manager = createManager();

    Region r = createValidRegion();
    manager.addRegion(r);
    assertNotNull(r.getRegionId());

    assertTrue(manager.getRegion(r.getRegionId()) != null);
    assertTrue(manager.getRegions().contains(r));
    assertTrue(manager.getRegion(r.getMasks().iterator().next()) != null);
    assertTrue(!manager.getRegionsBySmsc(r.getSmsc()).isEmpty());
    assertTrue(manager.containsRegionWithName(r.getName()));
  }

  @Test
  public void testRemoveRegion() throws AdminException, InitException {
    RegionsManager manager = createManager();
    Region r = getExistedRegion();

    manager.removeRegion(r.getRegionId());

    assertTrue(manager.getRegion(r.getRegionId()) == null);
    assertFalse(manager.getRegions().contains(r));
    assertTrue(manager.getRegion(r.getMasks().iterator().next()) == null);
    assertTrue(manager.getRegionsBySmsc(r.getSmsc()).isEmpty());
    assertFalse(manager.containsRegionWithName(r.getName()));
  }

  @Test
  public void testUpdateRegion() throws AdminException, InitException {
    RegionsManager manager = createManager();
    Region r = getExistedRegion();

    String oldName = r.getName();

    r.setName("MYREGION");
    r.setSmsc("MYSMSC");
    r.addMask(new Address("+79999999999"));

    manager.updateRegion(r);

    assertTrue(manager.getRegion(r.getRegionId()) != null);
    assertTrue(manager.getRegions().contains(r));
    assertTrue(manager.getRegion(new Address("+79999999999")) != null);
    assertTrue(manager.getRegionsBySmsc("MYSMSC").contains(r));
    assertTrue(manager.containsRegionWithName("MYREGION"));
    assertFalse(manager.containsRegionWithName(oldName));
    assertTrue(manager.getRegion(r.getRegionId()).getName().equals(r.getName()));
  }

  @Test
  public void testGetSetDefaultMaxPerSecond() throws Exception {
    RegionsManager manager = createManager();

    manager.setDefaultMaxPerSecond(143);
    assertEquals(143, manager.getDefaultMaxPerSecond());
  }

  @Test
  public void testContainsRegionWithName() throws Exception {
    RegionsManager manager = createManager();
    Region r = getExistedRegion();

    assertTrue(manager.containsRegionWithName(r.getName()));
    assertFalse(manager.containsRegionWithName("dfga"));
    assertFalse(manager.containsRegionWithName(null));
    assertFalse(manager.containsRegionWithName(""));
  }


  // ПРОВЕРКА ОСОБЕННОСТЕЙ РЕАЛИЗАЦИИ


  @Test
  public void testAddRollback() throws AdminException, InitException {

    RegionsManager regionsManager = createManager(true);
    Region r1 = new Region();
    r1.setName("МР СЕВЕР");
    r1.setMaxSmsPerSecond(3113);
    r1.setSmsc("SMSC_ERROR");
    r1.setTimeZone(TimeZone.getDefault());

    r1.addMask(new Address("+79039??????"));

    long beforeLenght = configFile.length();
    try {
      regionsManager.addRegion(r1);
      assertTrue(false);
    } catch (AdminException e) {
    }

    long afterLenght = configFile.length();
    assertEquals(beforeLenght, afterLenght);
    assertEquals(regionsManager.getRegionsBySmsc(r1.getSmsc()).size(), 0);
  }

  @Test
  public void testRemoveRollback() throws AdminException, InitException {
    RegionsManager regionsManager = createManager(true);

    Region r1 = regionsManager.getRegions().iterator().next();

    long beforeLenght = configFile.length();
    try {
      regionsManager.removeRegion(r1.getRegionId());
      assertTrue(false);
    } catch (AdminException e) {
    }

    long afterLenght = configFile.length();
    assertEquals(beforeLenght, afterLenght);
    assertTrue(regionsManager.getRegion(r1.getRegionId()) != null);
  }

  @Test
  public void testUpdateRollback() throws AdminException, InitException {
    RegionsManager regionsManager = createManager(true);

    Region r1 = regionsManager.getRegions().iterator().next();
    r1.setName(r1.getName() + "-");

    long beforeLenght = configFile.length();
    try {
      regionsManager.updateRegion(r1);
      assertTrue(false);
    } catch (AdminException e) {
    }

    long afterLenght = configFile.length();
    assertEquals(beforeLenght, afterLenght);
    assertTrue(regionsManager.getRegion(r1.getRegionId()) != null);
    assertTrue(regionsManager.getRegion(r1.getRegionId()).getName().equals(r1.getName().substring(0, r1.getName().length() - 1)));
  }

  @Test
  public void testPersistence() throws Exception {
    RegionsManager regionsManager = createManager();

    Region r = createValidRegion();
    regionsManager.addRegion(r);

    regionsManager = createManager();
    assertTrue(regionsManager.containsRegionWithName(r.getName()));
    assertTrue(regionsManager.getRegion(r.getRegionId()) != null);
    assertTrue(regionsManager.getRegionsBySmsc(r.getSmsc()).contains(r));

    regionsManager.removeRegion(r.getRegionId());

    regionsManager = createManager();
    assertFalse(regionsManager.containsRegionWithName(r.getName()));
    assertFalse(regionsManager.getRegion(r.getRegionId()) != null);
    assertFalse(regionsManager.getRegionsBySmsc(r.getSmsc()).contains(r));
  }
}
