package ru.novosoft.smsc.admin.archive_daemon;

import org.junit.*;
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
public class ArchiveDaemonManagerTest {

  private File configFile, backupDir;
  private ArchiveDaemonManager c;

  @Before
  public void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(ArchiveDaemonManagerTest.class.getResourceAsStream("config.xml"), ".archivedaemon");
    backupDir = TestUtils.createRandomDir(".archivedaemonbackup");
    c = new ArchiveDaemonManager(configFile, backupDir, FileSystem.getFSForSingleInst());
    c.reset();
  }

  @After
  public void afterClass() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  private void validateConfig(ArchiveDaemonManager c) {
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
  public void loadTest() throws AdminException {
    validateConfig(c);
  }

  @Test
  public void resetTest() throws AdminException {
    assertFalse(c.isChanged());

    c.setInterval(21);
    c.setQueriesMax(1);

    assertTrue(c.isChanged());

    c.reset();

    assertFalse(c.isChanged());

    validateConfig(c);    
  }

  @Test
  public void applyTest() throws XmlConfigException, AdminException {
    // Загружаем первоначальный конфиг
    XmlConfig cfg = new XmlConfig();
    cfg.load(configFile);

    // Создаем инстанц SmscConfig
    c.setInterval(30);
    // Сохраняем SmscConfig
    c.apply();

    // Проверяем, что в директории backup появились файлы
    assertFalse(backupDir.delete()); // Не можем удалить директорию т.к. там появились файлы

    // Снова загружаем конфиг
    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(configFile);

    // Проверяем эквивалентность первоначальному конфигу.
    assertEquals(cfg, cfg1);
  }

}
