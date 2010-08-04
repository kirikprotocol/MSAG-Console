package ru.novosoft.smsc.admin.resource;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerStub;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class ResourceManagerTest {

  private File configDir;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configDir = TestUtils.createRandomDir(".resource");
    TestUtils.exportResource(ResourceManagerTest.class.getResourceAsStream("resources_en_en.xml"), new File(configDir, "resources_en_en.xml"));
    TestUtils.exportResource(ResourceManagerTest.class.getResourceAsStream("resources_ru_ru.xml"), new File(configDir, "resources_ru_ru.xml"));
  }

  @After
  public void afterClass() {
    if (configDir != null)
      TestUtils.recursiveDeleteFolder(configDir);
  }

  private ResourceManager getManager(ClusterController cc) throws AdminException {
    return new ResourceManager(configDir, new File(configDir, "backup"), cc, FileSystem.getFSForSingleInst());
  }

  @Test
  public void getLocalesTest() throws AdminException {
    Collection<String> locales = getManager(new TestClusterControllerStub()).getLocales();

    assertTrue(locales.contains("ru_ru"));
    assertTrue(locales.contains("en_en"));    
  }

  @Test
  public void addResourceSettingsTest() throws AdminException {
    ResourceManager m = getManager(new TestClusterControllerStub());
    Resources r = new Resources();
    ResourceSettings s = new ResourceSettings("->", ";", r);
    m.addResourceSettings("ge_ge", s);

    assertTrue(new File(configDir, "resources_ge_ge.xml").exists());

    ResourceManager m1 = getManager(new TestClusterControllerStub());
    ResourceSettings s1 = m1.getResourceSettings("ge_ge");
    assertNotNull(s1);
    assertEquals("->", s1.getDecimalDelimiter());
    assertEquals(";", s1.getListDelimiter());
  }

  @Test
  public void removeResourceSettingsTest() throws AdminException {
    ResourceManager m = getManager(new TestClusterControllerStub());

    assertFalse(m.removeResourceSettings("ge_ge"));
    assertTrue(m.removeResourceSettings("en_en"));
    assertFalse(new File(configDir, "resources_en_en.xml").exists());

    assertFalse(m.getLocales().contains("en_en"));
  }  
}
