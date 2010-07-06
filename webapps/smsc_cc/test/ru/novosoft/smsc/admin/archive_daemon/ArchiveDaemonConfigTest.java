package ru.novosoft.smsc.admin.archive_daemon;

import junit.framework.AssertionFailedError;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.FileUtils;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;

import java.io.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;

/**
 * @author Artem Snopkov
 */
public class ArchiveDaemonConfigTest {

  private static File configFile, backupDir;

  @BeforeClass
  public static void beforeClass() throws IOException, AdminException {

    do {
      configFile = new File(System.currentTimeMillis() + ".xml");
      backupDir = new File(System.currentTimeMillis() + ".backup");
    } while (configFile.exists());

    InputStream is = null;
    OutputStream os = null;
    try {
      is = new BufferedInputStream(ArchiveDaemonConfigTest.class.getResourceAsStream("config.xml"));
      os = new BufferedOutputStream(new FileOutputStream(configFile));

      int b;
      while ((b = is.read()) >= 0)
        os.write(b);
    } catch (IOException e) {
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
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      FileUtils.recursiveDeleteFolder(backupDir);
  }

  @Test
  public void loadTest() throws AdminException {
    ArchiveDaemonConfig c = new ArchiveDaemonConfig(configFile, backupDir, FileSystem.getFSForSingleInst());

    assertEquals(4096000,c.getIndexatorMaxFlushSpeed());
    assertEquals(Integer.valueOf(8), c.getIndexatorSmeAddrChunkSizes().get("MAP_PROXY"));
    assertEquals(30,c.getInterval());
    assertEquals("@smsc.arc.dir@",c.getLocationsBaseDestination());
    assertEquals("@smsc.arctxt.dir@",c.getLocationsTextDestinations());
    assertEquals("@smsc.arcnew.dir@",c.getLocationsSources().get("src1"));
    assertEquals(1,c.getQueriesInit());
    assertEquals(5,c.getQueriesMax());
    assertEquals(20000,c.getTransactionsMaxSmsCount());
    assertEquals(60,c.getTransactionsMaxTimeInterval());
    assertEquals("@listen.host@",c.getViewHost());
    assertEquals(1212,c.getViewPort());
    assertEquals(10,c.getViewTimeout());
  }

  @Test
  public void saveTest() throws XmlConfigException, AdminException {
    // Загружаем первоначальный конфиг
    XmlConfig cfg = new XmlConfig();
    cfg.load(configFile);

    // Создаем инстанц SmscConfig
    ArchiveDaemonConfig config1 = new ArchiveDaemonConfig(configFile, backupDir, FileSystem.getFSForSingleInst());
    // Сохраняем SmscConfig
    config1.save();

    // Проверяем, что в директории backup появились файлы
    assertFalse(backupDir.delete()); // Не можем удалить директорию т.к. там появились файлы

    // Снова загружаем конфиг
    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(configFile);

    // Проверяем эквивалентность первоначальному конфигу.
    assertEquals(cfg, cfg1);
  }

}
