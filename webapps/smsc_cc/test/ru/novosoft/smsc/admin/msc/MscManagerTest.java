package ru.novosoft.smsc.admin.msc;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class MscManagerTest {

  private static File configFile;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(MscManagerTest.class.getResourceAsStream("msc.bin"), ".msc");
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
  }

  @Test(expected = IllegalArgumentException.class)
  public void addNullMscTest() throws AdminException {
    MscManager m = new MscManager(configFile, new TestClusterController(), FileSystem.getFSForSingleInst());
    m.addMsc(null);
  }

  @Test(expected = IllegalArgumentException.class)
  public void removeNullMscTest() throws AdminException {
    MscManager m = new MscManager(configFile, new TestClusterController(), FileSystem.getFSForSingleInst());
    m.removeMsc(null);
  }

  @Test
  public void getMscsTest() throws AdminException {
    MscManager m = new MscManager(configFile, new TestClusterController(), FileSystem.getFSForSingleInst());
    List<Address> mscs = new ArrayList<Address>(m.mscs());
    assertNotNull(mscs);
    assertEquals(3, mscs.size());
    assertEquals(new Address("79139495113"), mscs.get(0));
    assertEquals(new Address("6785"), mscs.get(1));
    assertEquals(new Address("34242"), mscs.get(2));
  }
}
