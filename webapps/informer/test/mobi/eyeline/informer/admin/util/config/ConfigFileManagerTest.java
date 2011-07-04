package mobi.eyeline.informer.admin.util.config;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.MemoryFileSystem;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;

import static junit.framework.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * User: artem
 * Date: 25.03.11
 */
public class ConfigFileManagerTest {

  MemoryFileSystem fs = new MemoryFileSystem();
  File configFile, backupDir;

  @Before
  public void setUp() throws Exception {
    configFile = new File("config.xml");
    backupDir = new File("backup");
  }

  @Test
  public void testCreate() {
    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new IntegerConfigFile());
    assertEquals(configFile, m.getConfigFile());
    assertEquals(fs, m.getFileSystem());
  }

  @Test
  public void testSaveAndLoad() throws Exception {
    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new IntegerConfigFile());

    m.save(100);

    assertEquals(new Integer(100), m.load());
  }

  @Test(expected=ConfigException.class)
  public void testSaveError() throws AdminException {
    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new ManagedConfigFile<Integer>() {
      @Override
      public void save(InputStream oldFile, OutputStream newFile, Integer conf) throws Exception {
        throw new ConfigException("io_error");
      }
      @Override
      public Integer load(InputStream is) throws Exception {
        return null;
      }
    });

    m.save(100);
  }

  @Test
  public void testNoBackupCreationIfConfigFileDoesNotExists() throws Exception {
    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new IntegerConfigFile());

    m.save(100); // При первом сохранении бекап не создается

    assertTrue(fs.exists(backupDir));
    assertTrue(fs.list(backupDir).length == 0);
  }

  @Test
  public void testBackupCreationIfConfigFileExists() throws Exception {
    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new IntegerConfigFile());

    m.save(100); // При первом сохранении бекап не создается
    m.save(200); // При втором сохранении создается бекап

    assertTrue(fs.exists(backupDir));
    assertTrue(fs.list(backupDir).length == 1);


  }

  @Test
  public void testRollbackFromBackupFileThatDoesNotExists() throws Exception {
    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new IntegerConfigFile());

    File backupFile = new File(backupDir, "file_does_not_exists");

    assertFalse(m.rollback(backupFile));
  }

  @Test
  public void testRollbackFromNotEmptyBackup() throws Exception {
    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new IntegerConfigFile());

    m.save(100);
    m.save(200);

    assertTrue(fs.exists(backupDir));
    assertTrue(fs.list(backupDir).length == 1);

    File backupFile = new File(backupDir, fs.list(backupDir)[0]);

    assertEquals(fs.length(configFile), fs.length(backupFile));

    assertTrue(m.rollback(backupFile));

    assertEquals(new Integer(100), m.load());
  }

  @Test(expected=ConfigException.class)
  public void testLoadError() throws AdminException {

    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new ManagedConfigFile<Integer>() {
      @Override
      public void save(InputStream oldFile, OutputStream newFile, Integer conf) throws Exception {
      }
      @Override
      public Integer load(InputStream is) throws Exception {
        throw new ConfigException("io_error");
      }
    });

    fs.createNewFile(configFile);

    m.load();
  }

  @Test(expected=AdminException.class)
  public void testLoadFromFileThatDoesNotExists() throws AdminException {

    ConfigFileManager<Integer> m = new ConfigFileManager<Integer>(configFile, backupDir, fs, new IntegerConfigFile());

    m.load();
  }

  private class IntegerConfigFile implements ManagedConfigFile<Integer> {

    @Override
    public void save(InputStream oldFile, OutputStream newFile, Integer conf) throws Exception {
      newFile.write(conf);
    }

    @Override
    public Integer load(InputStream is) throws Exception {
      return is.read();
    }
  }
}
