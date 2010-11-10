package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.InfosmeException;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import mobi.eyeline.informer.util.Address;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.util.TimeZone;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class RegionsManagerTest {

  private static File configFile, backupDir;

  private RegionsManager regionsManager;

  private boolean infosmeError = false;

  @BeforeClass
  public static void init() throws Exception{
    configFile = TestUtils.exportResourceToRandomFile(RegionsManager.class.getResourceAsStream("regions.xml"), ".regions");
    backupDir = TestUtils.createRandomDir(".config.backup");
  }

  @Before
  public void before() throws Exception {
    Infosme infosem = new TestInfosme(){
      @Override
      public void addRegion(int regionId) throws AdminException {
        if(infosmeError) {
          throw new InfosmeException("interaction_error","");
        }
        super.addRegion(regionId);
      }

      @Override
      public void updateRegion(int regionId) throws AdminException {
        if(infosmeError) {
          throw new InfosmeException("interaction_error","");
        }
        super.updateRegion(regionId);
      }

      @Override
      public void removeRegion(int regionId) throws AdminException {
        if(infosmeError) {
          throw new InfosmeException("interaction_error","");
        }
        super.removeRegion(regionId);
      }
      
    };
    regionsManager = new RegionsManager(infosem, configFile, backupDir, FileSystem.getFSForSingleInst());
    for(Region s : regionsManager.getRegions()) {
      infosem.addRegion(s.getRegionId());
    }
  }



  @Test
  public void update() throws Exception {
    Region region = regionsManager.getRegion(new Address("+79139??????"));
    assertTrue(region != null);
    assertEquals(region.getName(), "МР Москва");
    assertEquals(region.getTimeZone(), TimeZone.getTimeZone("Europe/Moscow"));
    assertEquals(region.getMaxSmsPerSecond(), 200);
    assertEquals(region.getSmsc(), "SMSC2");
    assertEquals(region.getMasks().size(), 2);

    for(Address a : region.getMasks()) {
      assertTrue(a.getSimpleAddress().equals("+79139??????") || a.getSimpleAddress().equals("+79137??????"));
    }

    region.setName("МР Москва 2");

    regionsManager.updateRegion(region);

    before();

    Region r1 = regionsManager.getRegion(region.getRegionId());

    assertEquals(r1.getName(), "МР Москва 2");
    assertEquals(r1.getTimeZone(), TimeZone.getTimeZone("Europe/Moscow"));
    assertEquals(r1.getMaxSmsPerSecond(), 200);
    assertEquals(r1.getSmsc(), "SMSC2");
    assertEquals(r1.getMasks().size(), 2);

    for(Address a : r1.getMasks()) {
      assertTrue(a.getSimpleAddress().equals("+79139??????") || a.getSimpleAddress().equals("+79137??????"));
    }
  }

  @Test
  public void addRemove() throws Exception {

    Region r1 = new Region();

    r1.setName("МР СИБИРЬ");
    r1.setMaxSmsPerSecond(3113);
    r1.setSmsc("SMSC3");
    r1.setTimeZone(TimeZone.getDefault());

    r1.addMask(new Address("+79239??????"));

    regionsManager.addRegion(r1);

    before();

    r1 = regionsManager.getRegion(r1.getMasks().iterator().next());
    assertEquals(r1.getName(), "МР СИБИРЬ");
    assertEquals(r1.getTimeZone(), TimeZone.getDefault());
    assertEquals(r1.getMaxSmsPerSecond(), 3113);
    assertEquals(r1.getSmsc(), "SMSC3");
    assertEquals(r1.getMasks().size(), 1);

    assertTrue(r1.getMasks().iterator().next().getSimpleAddress().equals("+79239??????"));

    regionsManager.removeRegion(r1.getRegionId());

    before();

    r1 = regionsManager.getRegion(r1.getRegionId());
    assertTrue(r1 == null);
  }

  @Test
  public void setDef() throws Exception {

    int newDef = regionsManager.getDefaultMaxPerSecond() != 532 ? 532 : 674;

    regionsManager.setDefaultMaxPerSecond(newDef);

    before();

    assertTrue(regionsManager.getDefaultMaxPerSecond() == newDef);
  }


  @Test
  public void testAddRollback() throws AdminException {
    try{
      infosmeError = true;
      Region r1 = new Region();
      r1.setName("МР СЕВЕР");
      r1.setMaxSmsPerSecond(3113);
      r1.setSmsc("SMSC_ERROR");
      r1.setTimeZone(TimeZone.getDefault());

      r1.addMask(new Address("+79039??????"));

      long beforeLenght = configFile.length();
      try{
        regionsManager.addRegion(r1);
        assertTrue(false);
      }catch (AdminException e){}

      long afterLenght = configFile.length();
      assertEquals(beforeLenght, afterLenght);
      assertEquals(regionsManager.getRegionsBySmsc(r1.getSmsc()).size(), 0);
    }finally {
      infosmeError = false;
    }
  }

  @Test
  public void testRemoveRollback() throws AdminException {
    try{
      infosmeError = true;

      Region r1 = regionsManager.getRegions().iterator().next();

      long beforeLenght = configFile.length();
      try{
        regionsManager.removeRegion(r1.getRegionId());
        assertTrue(false);
      }catch (AdminException e){}

      long afterLenght = configFile.length();
      assertEquals(beforeLenght, afterLenght);
      assertTrue(regionsManager.getRegion(r1.getRegionId()) != null);
    }finally {
      infosmeError = false;
    }
  }

  @Test
  public void testUpdateRollback() throws AdminException {
    try{
      infosmeError = true;

      Region r1 = regionsManager.getRegions().iterator().next();
      r1.setName(r1.getName()+"-");

      long beforeLenght = configFile.length();
      try{
        regionsManager.updateRegion(r1);
        assertTrue(false);
      }catch (AdminException e){}

      long afterLenght = configFile.length();
      assertEquals(beforeLenght, afterLenght);
      assertTrue(regionsManager.getRegion(r1.getRegionId()) != null);
      assertTrue(regionsManager.getRegion(r1.getRegionId()).getName().equals(r1.getName().substring(0, r1.getName().length()-1)));
    }finally {
      infosmeError = false;
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
