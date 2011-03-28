package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import mobi.eyeline.informer.admin.util.validation.ValidationException;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

/**
 * User: artem
 * Date: 28.03.11
 */
public class RestrictionOperationsTest {

  private MemoryFileSystem fs;
  private File configFile;
  private File backupDir;

  private List<RestrictionProvider> providers;

  @Before
  public void setUp() {
    fs = new MemoryFileSystem();
    configFile = new File("restrictions.csv");
    backupDir = new File("backup");
    providers = new ArrayList<RestrictionProvider>();
  }

  private RestrictionProvider createProvider() throws AdminException, InitException {
    RestrictionProvider p =  new RestrictionProvider(new RestrictionContextStub(), configFile, backupDir, fs);
    providers.add(p);
    return p;
  }

  @After
  public void tearDown() {
    for (RestrictionProvider p : providers)
      p.shutdown();
  }

  private void createConfigFile(String... lines) throws AdminException {
    PrintWriter os = null;
    try {
      os = new PrintWriter(new OutputStreamWriter(fs.getOutputStream(configFile, false)));
      for (String line : lines)
        os.println(line);
    } finally {
      if (os != null)
        os.close();
    }
  }

  private static String restrictionToString(Restriction r) {
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
    String line = r.getId() + "," + r.getName() + ',' + sdf.format(r.getStartDate()) + ',' + sdf.format(r.getEndDate()) + ',' + r.isAllUsers();
    for (String uId : r.getUserIds())
      line+=',' + uId;
    return line;
  }

  private void createConfigFile(Restriction ... restrictions) throws AdminException {
    String[] lines = new String[restrictions.length];


    for (int i=0; i<restrictions.length; i++) {
      Restriction r = restrictions[i];
      lines[i] = restrictionToString(r);
    }
    createConfigFile(lines);
  }

  private static Restriction createRestriction(int id, String name, String from, String till, boolean all, String... users) throws Exception{
    Restriction r = new Restriction();
    r.setId(id);
    r.setName(name);
    r.setStartDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse(from));
    r.setEndDate(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").parse(till));
    r.setAllUsers(all);
    r.setUserIds(Arrays.asList(users));
    return r;
  }

  private static void assertRestrictionsEquals(Restriction expected, Restriction val) {
    assertEquals(expected.getId(), val.getId());
    assertEquals(expected.getName(), val.getName());
    assertEquals(expected.getStartDate(), val.getStartDate());
    assertEquals(expected.getEndDate(), val.getEndDate());
    assertEquals(expected.isAllUsers(), val.isAllUsers());
    if (!expected.isAllUsers()) {
      List<String> u1 = expected.getUserIds();
      List<String> u2 = val.getUserIds();
      assertEquals(u1.size(), u2.size());
      for (int i=0; i<u1.size(); i++)
        assertEquals(u1.get(i), u2.get(i));
    }
  }

  private RestrictionsFilter createFilter(String name, String from, String till, String user) throws Exception {
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
    RestrictionsFilter f = new RestrictionsFilter();
    f.setNameFilter(name);
    if (from != null)
      f.setStartDate(sdf.parse(from));
    if (till != null)
      f.setEndDate(sdf.parse(till));
    f.setUserId(user);
    return f;
  }
  
  @Test
  public void testCreateWhenConfigFileDoesNotExists() throws AdminException, InitException {
    createProvider();
  }

  @Test(expected=InitException.class)
  public void testInvalidFile() throws Exception {
    createConfigFile(
        "invalid line, ,edf , as"
    );
    createProvider();
  }

  @Test
  public void testSaveAndLoadRestrictions() throws Exception {
    Restriction r1, r2;
    createConfigFile(
        r1 = createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true),
        r2 = createRestriction(2, "test restriction 2", "2010.12.30 00:00:00", "2011.01.05 00:00:00", false, "b", "a")
    );

    RestrictionProvider p = createProvider();

    List<Restriction> restrictions = p.getRestrictions(new RestrictionsFilter());
    assertNotNull(restrictions);
    assertEquals(2, restrictions.size());

    assertRestrictionsEquals(r1, restrictions.get(0));
    assertRestrictionsEquals(r2, restrictions.get(1));
  }

  @Test
  public void testLoadRestrictionsFromEmptyFile() throws Exception {
    fs.createNewFile(configFile);

    RestrictionProvider p = createProvider();

    List<Restriction> restrictions = p.getRestrictions(new RestrictionsFilter());
    assertEquals(0, restrictions.size());
  }

  @Test
  public void testGetretrictionById() throws Exception {
    Restriction r1, r2;
    createConfigFile(
        r1 = createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true),
        r2 = createRestriction(2, "test restriction 2", "2010.12.30 00:00:00", "2011.01.05 00:00:00", false, "b", "a")
    );

    RestrictionProvider p = createProvider();
    assertRestrictionsEquals(r1, p.getRestriction(1));
    assertRestrictionsEquals(r2, p.getRestriction(2));
  }

  @Test
  public void testFilterByName() throws Exception {
    Restriction r1, r2;
    createConfigFile(
        r1 = createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true),
        r2 = createRestriction(2, "test restriction 2", "2010.12.30 00:00:00", "2011.01.05 00:00:00", false, "b", "a")
    );

    RestrictionProvider p = createProvider();

    List<Restriction> restrictions = p.getRestrictions(createFilter("test", null, null, null));  // Префикс
    assertEquals(2, restrictions.size());
    assertRestrictionsEquals(r1, restrictions.get(0));
    assertRestrictionsEquals(r2, restrictions.get(1));

    restrictions = p.getRestrictions(createFilter("test restriction 1", null, null, null)); // Точное совпадение
    assertEquals(1, restrictions.size());
    assertRestrictionsEquals(r1, restrictions.get(0));

    restrictions = p.getRestrictions(createFilter("unknown", null, null, null)); // Полное несовпадение
    assertEquals(0, restrictions.size());
  }

  @Test
  public void testFilterByStartDate() throws Exception {
    Restriction r1, r2;
    createConfigFile(
        r1 = createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true),
        r2 = createRestriction(2, "test restriction 2", "2010.12.30 00:00:00", "2011.01.05 00:00:00", false, "b", "a")
    );

    RestrictionProvider p = createProvider();

    List<Restriction> restrictions = p.getRestrictions(createFilter(null, "2010.01.02 00:00:00", null, null));
    assertEquals(2, restrictions.size());
    assertRestrictionsEquals(r1, restrictions.get(0));
    assertRestrictionsEquals(r2, restrictions.get(1));

    restrictions = p.getRestrictions(createFilter(null, "2010.11.30 00:00:00", null, null));
    assertEquals(1, restrictions.size());
    assertRestrictionsEquals(r2, restrictions.get(0));

    restrictions = p.getRestrictions(createFilter(null, "2011.12.30 00:00:00", null, null));
    assertEquals(0, restrictions.size());
  }

  @Test
  public void testFilterByEndDate() throws Exception {
    Restriction r1, r2;
    createConfigFile(
        r1 = createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true),
        r2 = createRestriction(2, "test restriction 2", "2010.12.30 00:00:00", "2011.01.05 00:00:00", false, "b", "a")
    );

    RestrictionProvider p = createProvider();

    List<Restriction> restrictions = p.getRestrictions(createFilter(null, null, "2011.01.05 00:00:00", null));
    assertEquals(2, restrictions.size());
    assertRestrictionsEquals(r1, restrictions.get(0));
    assertRestrictionsEquals(r2, restrictions.get(1));

    restrictions = p.getRestrictions(createFilter(null, null, "2010.01.06 00:00:00", null));
    assertEquals(1, restrictions.size());
    assertRestrictionsEquals(r1, restrictions.get(0));

    restrictions = p.getRestrictions(createFilter(null, null, "2010.01.01 00:00:00", null));
    assertEquals(0, restrictions.size());
  }

  @Test
  public void testFilterByUser() throws Exception {
    Restriction r1, r2;
    createConfigFile(
        r1 = createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true),
        r2 = createRestriction(2, "test restriction 2", "2010.12.30 00:00:00", "2011.01.05 00:00:00", false, "b")
    );

    RestrictionProvider p = createProvider();

    List<Restriction> restrictions = p.getRestrictions(createFilter(null, null, null, "b"));
    assertEquals(2, restrictions.size());
    assertRestrictionsEquals(r1, restrictions.get(0));
    assertRestrictionsEquals(r2, restrictions.get(1));

    restrictions = p.getRestrictions(createFilter(null, null, null, "a"));
    assertEquals(1, restrictions.size());
    assertRestrictionsEquals(r1, restrictions.get(0));

  }

  @Test
  public void testAddRestriction() throws Exception {
    RestrictionProvider p = createProvider();

    Restriction r = createRestriction(10, "name", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true);
    p.addRestriction(r);

    p = createProvider();
    List<Restriction> restrictions = p.getRestrictions(new RestrictionsFilter());

    assertEquals(1, restrictions.size());
    assertRestrictionsEquals(r, restrictions.get(0));
    assertRestrictionsEquals(r, p.getRestriction(r.getId()));
  }

  @Test(expected=NullPointerException.class)
  public void testAddNullRestriction() throws Exception {
    RestrictionProvider p = createProvider();
    p.addRestriction(null);
  }

  @Test(expected=AdminException.class)
  public void testAddInvalidRestriction() throws Exception {
    RestrictionProvider p = createProvider();
    p.addRestriction(createRestriction(-1, null, "2010.01.02 00:00:00", "2010.01.05 00:00:00", true));
  }

  @Test
  public void testDeleteRestriction() throws Exception {
    Restriction r1, r2;
    createConfigFile(
        r1 = createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true),
        r2 = createRestriction(2, "test restriction 2", "2010.12.30 00:00:00", "2011.01.05 00:00:00", false, "b", "a")
    );

    RestrictionProvider p = createProvider();
    p.deleteRestriction(r1.getId());

    p = createProvider();
    List<Restriction> restrictions = p.getRestrictions(new RestrictionsFilter());
    assertNotNull(restrictions);
    assertEquals(1, restrictions.size());

    assertRestrictionsEquals(r2, restrictions.get(0));
  }

  @Test
  public void testUpdateRestriction() throws Exception {
    RestrictionProvider p = createProvider();
    p.addRestriction(createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true));

    Restriction r = createRestriction(1, "test restriction", "2011.01.02 00:00:00", "2011.01.05 00:00:00", false, "b", "a");
    p.updateRestriction(r);

    p = createProvider();
    assertRestrictionsEquals(r, p.getRestrictions(new RestrictionsFilter()).get(0));
  }

  @Test(expected=NullPointerException.class)
  public void testUpdateNullRestriction() throws Exception {
    RestrictionProvider p = createProvider();
    p.updateRestriction(null);
  }

  @Test(expected=AdminException.class)
  public void testUpdateInvalidRestriction() throws Exception {
    RestrictionProvider p = createProvider();
    p.addRestriction(createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true));

    Restriction r = createRestriction(1, null, "2011.01.02 00:00:00", "2011.01.05 00:00:00", false, "b", "a");
    p.updateRestriction(r);
  }

  @Test(expected=AdminException.class)
  public void testUpdateUnknownRestriction() throws Exception {
    RestrictionProvider p = createProvider();
    p.updateRestriction(createRestriction(1, "test restriction 1", "2010.01.02 00:00:00", "2010.01.05 00:00:00", true));
  }

}
