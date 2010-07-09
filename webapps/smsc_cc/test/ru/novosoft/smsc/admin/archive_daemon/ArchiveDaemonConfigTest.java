package ru.novosoft.smsc.admin.archive_daemon;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class ArchiveDaemonConfigTest {

  private static File configFile, backupDir;

  @BeforeClass
  public static void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(ArchiveDaemonConfigTest.class.getResourceAsStream("config.xml"), ".archivedaemon");
    backupDir = TestUtils.createRandomDir(".archivedaemonbackup");
  }

  @AfterClass
  public static void afterClass() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
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

  @Test
  public void isChangedTest() throws AdminException {
    ArchiveDaemonConfig config1 = new ArchiveDaemonConfig(configFile, backupDir, FileSystem.getFSForSingleInst());
    assertFalse(config1.isChanged());
    config1.setInterval(config1.getInterval());
    assertTrue(config1.isChanged());
    config1.save();
    assertFalse(config1.isChanged());
    config1.setInterval(config1.getInterval());
    assertTrue(config1.isChanged());
    config1.reset();
    assertFalse(config1.isChanged());
  }

}
