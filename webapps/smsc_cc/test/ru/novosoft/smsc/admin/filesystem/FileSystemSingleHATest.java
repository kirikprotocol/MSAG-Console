package ru.novosoft.smsc.admin.filesystem;

import org.junit.Test;
import junit.framework.AssertionFailedError;
import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.FileUtils;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class FileSystemSingleHATest {

  FileSystem fs;
  File baseDir;

  @Before
  public void before() {
    File f;
    do {
      f = new File(System.currentTimeMillis() + "");
    } while (f.exists());

    if (!f.mkdirs())
      throw new AssertionFailedError("Unable to init FS");

    baseDir = f;

    fs = new FileSystemSingleHA();
  }

  @After
  public void after() {
    FileUtils.recursiveDeleteFolder(baseDir);
  }

  private void createRandomFile(File file) throws AdminException, IOException {
    OutputStream os = null;
    try {
      os = fs.getOutputStream(file);

      for (int i = 0; i < 1000; i++)
        os.write(i);

    } finally {
      if (os != null)
        os.close();
    }
  }

  @Test
  public void testGetOutputStream() throws Exception {
    File f = new File(baseDir, "test");

    assertFalse(f.exists());

    createRandomFile(f);

    assertTrue(f.exists());
    assertTrue(f.length() > 0);
  }

  @Test
  public void testRename() throws Exception {
    File f = new File(baseDir, "test");
    File fcopy = new File(baseDir, "test.copy");
    createRandomFile(f);

    fs.rename(f, fcopy);

    assertFalse(f.exists());
    assertTrue(fcopy.exists());
  }

  @Test
  public void testCopy() throws Exception {
    File f = new File(baseDir, "test");
    File fcopy = new File(baseDir, "test.copy");
    createRandomFile(f);

    fs.copy(f, fcopy);

    assertTrue(f.exists());
    assertTrue(fcopy.exists());
    assertTrue(fcopy.length() > 0);
  }

  @Test
  public void testDelete() throws Exception {
    File f = new File(baseDir, "test");

    createRandomFile(f);

    fs.delete(f);

    assertFalse(f.exists());
  }

  @Test
  public void testMkdirs() throws Exception {
    File f = new File(baseDir, "test");

    fs.mkdirs(f);

    assertTrue(f.exists());
  }

  @Test
  public void testExists() throws Exception {
    File f = new File(baseDir, "test");

    assertFalse(fs.exists(f));

    fs.mkdirs(f);

    assertTrue(fs.exists(f));
  }
}
