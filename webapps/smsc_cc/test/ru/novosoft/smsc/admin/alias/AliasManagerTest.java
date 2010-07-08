package ru.novosoft.smsc.admin.alias;

import junit.framework.AssertionFailedError;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.*;
import java.util.Locale;
import java.util.Properties;
import java.util.PropertyResourceBundle;
import java.util.ResourceBundle;

/**
 * @author Artem Snopkov
 */
public class AliasManagerTest {

  private static File aliasesFile;

  @BeforeClass
  public static void beforeClass() throws IOException {

    do {
      aliasesFile = new File(System.currentTimeMillis() + ".bin");
    } while (aliasesFile.exists());

    InputStream is = null;
    OutputStream os = null;
    try {
      is = new BufferedInputStream(AliasManagerTest.class.getResourceAsStream("aliases.bin"));
      os = new BufferedOutputStream(new FileOutputStream(aliasesFile));

      int b;
      while ((b = is.read()) >= 0)
        os.write(b);
    } catch (EOFException e) {
    } catch (Exception e) {
      e.printStackTrace();
      throw new AssertionFailedError(e.getMessage());
    } finally {
      if (is != null)
        is.close();
      if (os != null)
        os.close();
    }

  }

  @AfterClass
  public static void afterClass() {
    if (aliasesFile != null)
      aliasesFile.delete();
  }

  @Test
  public void testGetAliases() throws Exception {
    ClusterController cc = new ClusterController() {
      public File getAliasesFile() {
        return aliasesFile;
      }
    };

    AliasManager manager = new AliasManager(cc, FileSystem.getFSForSingleInst());

    AliasSet set = null;
    try {
      set = manager.getAliases();

      assertTrue(set.next());

      Alias alias1 = set.get();
      assertEquals(alias1.getAddress(), new Address(0,1,"31"));
      assertEquals(alias1.getAlias(), new Address(0,1,"123"));
      assertEquals(alias1.isHide(), false);

      assertTrue(set.next());

      Alias alias2 = set.get();
      assertEquals(alias2.getAddress(), new Address(0,1,"112233"));
      assertEquals(alias2.getAlias(), new Address(0,1,"332211"));
      assertEquals(alias2.isHide(), true);

      assertTrue(set.next());

      Alias alias3 = set.get();
      assertEquals(alias3.getAddress(), new Address(0,1,"223344"));
      assertEquals(alias3.getAlias(), new Address(0,1,"443322"));
      assertEquals(alias3.isHide(), false);

      assertFalse(set.next());

    } finally {
      if (set != null)
        set.close();
    }
  }
}
