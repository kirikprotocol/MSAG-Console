package ru.novosoft.smsc.admin.sme;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class SmeConfigFileTest {


  private File configFile;

  @Before
  public void before() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(SmeConfigFileTest.class.getResourceAsStream("sme.xml"), ".sme");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
  }

  private Map<String, Sme> loadSmes() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new SmeConfigFile());
  }

  private void saveSmes(Map<String, Sme> s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new SmeConfigFile(), s);
  }

  @Test
  public void testSave() throws Exception {
    Map<String, Sme> smes = loadSmes();
    saveSmes(smes);

    testLoad();
  }

  @Test
  public void testLoad() throws Exception {

    Map<String, Sme> smes = loadSmes();

    Sme DSTRLIST = smes.get("DSTRLST");
    assertNotNull(DSTRLIST);

    assertEquals(16384, DSTRLIST.getPriority());
    assertEquals("internal", DSTRLIST.getSystemType());
    assertEquals("", DSTRLIST.getPassword());
    assertEquals(".*", DSTRLIST.getAddrRange());
    assertEquals(-1, DSTRLIST.getSmeN());
    assertEquals(false, DSTRLIST.isWantAlias());
    assertEquals(20, DSTRLIST.getTimeout());
    assertEquals("default", DSTRLIST.getReceiptSchemeName());
    assertEquals(false, DSTRLIST.isDisabled());
    assertEquals(SmeBindMode.TRX, DSTRLIST.getBindMode());
    assertEquals(1, DSTRLIST.getAccessMask());
    assertEquals(true, DSTRLIST.isCarryOrgDescriptor());

  }
}
