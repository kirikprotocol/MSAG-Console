package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.InfosmeException;
import mobi.eyeline.informer.admin.infosme.TestInfosme;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

import static junit.framework.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 03.11.2010
 * Time: 19:05:12
 */
public class RestrictionManagerTest {

  private static File configFile, backupDir;

  private RestrictionsManager restrictionsManager;

  private boolean infosmeError = false;

  @BeforeClass
  public static void init() throws Exception{
    configFile = TestUtils.exportResourceToRandomFile(RestrictionsManager.class.getResourceAsStream("restrictions.csv"), ".restrictions");
    backupDir = TestUtils.createRandomDir(".config.backup");
  }

  @Before
  public void before() throws Exception {
    Infosme infosem = new TestInfosme(){
      @Override
      public void addRegion(String regionId) throws AdminException {
        if(infosmeError) {
          throw new InfosmeException("interaction_error","");
        }
        super.addRegion(regionId);
      }

      @Override
      public void updateRegion(String regionId) throws AdminException {
        if(infosmeError) {
          throw new InfosmeException("interaction_error","");
        }
        super.updateRegion(regionId);
      }

      @Override
      public void removeRegion(String regionId) throws AdminException {
        if(infosmeError) {
          throw new InfosmeException("interaction_error","");
        }
        super.removeRegion(regionId);
      }

    };
    restrictionsManager = new TestRestrictionsManager(infosem, configFile, backupDir, FileSystem.getFSForSingleInst());
  }

  @Test
  public void testLoad() {
    List<Restriction> result = restrictionsManager.getRestrictions(null);
    assertTrue(result.size()==2);

    Restriction r = restrictionsManager.getRestriction(1);
    assertTrue(r.getId()==1);
    assertTrue(r.getName().equals("test restriction 1"));
    try {
      assertTrue(r.getStartDate().equals(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2010.01.02 00:00:00")));
      assertTrue(r.getEndDate().equals(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2010.01.05 00:00:00")));
    }
    catch (Exception e) {
      assertTrue(false);
    }
    assertTrue(r.isAllUsers());
    assertTrue(r.getUserIds()==null);


    r = restrictionsManager.getRestriction(2);
    assertTrue(r.getId()==2);
    assertTrue(r.getName().equals("test restriction 2"));
    try {
      assertTrue(r.getStartDate().equals(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2010.12.30 00:00:00")));
      assertTrue(r.getEndDate().equals(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2011.01.05 00:00:00")));
    }
    catch (Exception e) {
      assertTrue(false);
    }
    assertTrue(!r.isAllUsers());
    assertTrue(r.getUserIds().size()==2);
  }

  @Test
  public void testInvalidParams() throws AdminException, ParseException {
    Restriction r = new Restriction();

    //check add
    r.setName("hello");
    r.setStartDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2011.02.05 11:30:00"));
    r.setEndDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2011.02.05 12:30:00"));
    r.setAllUsers(true);
    restrictionsManager.addRestriction(r);


    //name == null
    try {
      r.setName(null);
      restrictionsManager.updateRestriction(r);
      assertTrue(false);
    }
    catch (Exception e) {
    }

    //endDate == null
    try {
      r.setName("hello");
      r.setEndDate(null);
      restrictionsManager.updateRestriction(r);
      assertTrue(false);
    }
    catch (Exception e) {
    }

    //startDate==null
    try {
      r.setStartDate(null);
      r.setEndDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2011.02.05 12:30:00"));
      restrictionsManager.updateRestriction(r);
      assertTrue(false);
    }
    catch (Exception e) {
    }

    //startDate > endDate
    try {
      r.setStartDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2011.03.05 11:30:00"));
      restrictionsManager.updateRestriction(r);
      assertTrue(false);
    }
    catch (Exception e) {
    }

    //allUsers with null users list
    try {
      r.setStartDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse("2011.02.05 11:30:00"));
      r.setAllUsers(false);
      restrictionsManager.updateRestriction(r);
      assertTrue(false);
    }
    catch (Exception e) {
    }

    //successfully update
    List<String> uids = new ArrayList<String>();
    uids.add("b");
    r.setUserIds(uids);
    restrictionsManager.updateRestriction(r);

    //check users list contains b after update
    assertTrue(restrictionsManager.getRestrictions(null).size()==3);
    r = restrictionsManager.getRestriction(3);
    assertTrue(r.getUserIds().size()==1);
    assertTrue(r.getUserIds().contains("b"));
    assertTrue(!r.isAllUsers());

    //check delete
    restrictionsManager.deleteRestriction(1);
    List<Restriction> result = restrictionsManager.getRestrictions(null);
    for(Restriction rr : result) {
      assertTrue(rr.getId()!=1);
    }

  }


}
