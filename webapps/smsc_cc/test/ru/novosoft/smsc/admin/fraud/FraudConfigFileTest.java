package ru.novosoft.smsc.admin.fraud;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ManagedConfigHelper;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * @author Artem Snopkov
 */
public class FraudConfigFileTest {

  private File configFile;

  @Before
  public void before() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(FraudManagerImplTest.class.getResourceAsStream("fraud.xml"), ".fraud");
  }

  @After
  public void after() {
    if (configFile != null)
      configFile.delete();
  }
  
  private FraudSettings loadSettings() throws Exception {
    return ManagedConfigHelper.loadConfig(configFile, new FraudConfigFile());
  }

  private void saveSettings(FraudSettings s) throws Exception {
    ManagedConfigHelper.saveConfig(configFile, new FraudConfigFile(), s);
  }
  
  @Test
  public void testSave() throws Exception {
    FraudSettings settings1 = loadSettings();

    settings1.setTail(5);
    settings1.setEnableCheck(false);
    settings1.setEnableReject(true);
    Collection<Address> whiteList = new ArrayList<Address>();
    Collections.addAll(whiteList, new Address("23123"), new Address("4324"), new Address("5564"));
    settings1.setWhiteList(whiteList);

    saveSettings(settings1);

    FraudSettings settings = loadSettings();

    assertEquals(5, settings.getTail());
    assertFalse(settings.isEnableCheck());
    assertTrue(settings.isEnableReject());
    assertEquals(3, settings.getWhiteList().size());
    assertTrue(settings.getWhiteList().contains(new Address("23123")));
    assertTrue(settings.getWhiteList().contains(new Address("4324")));
    assertTrue(settings.getWhiteList().contains(new Address("5564")));
  }

  @Test
  public void testLoad() throws Exception {

    FraudSettings fm = loadSettings();

    assertEquals(1, fm.getTail());
    assertTrue(fm.isEnableCheck());
    assertFalse(fm.isEnableReject());
    assertEquals(2, fm.getWhiteList().size());
    assertTrue(fm.getWhiteList().contains(new Address("12232321")));
    assertTrue(fm.getWhiteList().contains(new Address("9139495113")));
  }
}
