package mobi.eyeline.informer.admin.filesystem;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class FileSystemHSTest {

  private FileSystem fs;
  private File base, mirror;

  @Before
  public void before() {
    base = TestUtils.createRandomDir(".base");
    mirror = TestUtils.createRandomDir(".mirror");

    fs = new FileSystemHS(base, mirror);
  }

  @After
  public void after() {
    TestUtils.recursiveDeleteFolder(base);
    TestUtils.recursiveDeleteFolder(mirror);
  }

  private void createRandomFile(File file) throws AdminException, IOException {
    OutputStream os = null;
    try {
      os = fs.getOutputStream(file, false);

      for (int i = 0; i < 1000; i++)
        os.write(i);

    } finally {
      if (os != null)
        os.close();
    }

  }

  @Test
  public void testGetOutputStream() throws Exception {
    File baseFile = new File(base, "testFile");
    File mirrorFile = new File(mirror, "testFile");

    assertFalse(baseFile.exists());
    assertFalse(mirrorFile.exists());

    createRandomFile(baseFile);

    assertTrue(baseFile.exists());
    assertTrue(baseFile.length() > 0);

    assertTrue(mirrorFile.exists());
    assertTrue(mirrorFile.length() > 0);
  }

  @Test
  public void testRename() throws Exception {
    File baseFile = new File(base, "testFile");
    File mirrorFile = new File(mirror, "testFile");

    assertFalse(baseFile.exists());
    assertFalse(mirrorFile.exists());

    createRandomFile(baseFile);

    assertTrue(baseFile.exists());
    assertTrue(mirrorFile.exists());

    File renamedBaseFile = new File(baseFile.getAbsolutePath() + ".renamed");
    File renamedMirrorFile = new File(mirrorFile.getAbsolutePath() + ".renamed");

    fs.rename(baseFile, renamedBaseFile);

    assertFalse(baseFile.exists());
    assertFalse(mirrorFile.exists());

    assertTrue(renamedBaseFile.exists());
    assertTrue(renamedMirrorFile.exists());
  }

  @Test
  public void testCopy() throws Exception {
    File baseFile = new File(base, "testFile");
    File mirrorFile = new File(mirror, "testFile");

    assertFalse(baseFile.exists());
    assertFalse(mirrorFile.exists());

    createRandomFile(baseFile);

    assertTrue(baseFile.exists());
    assertTrue(mirrorFile.exists());

    File renamedBaseFile = new File(baseFile.getAbsolutePath() + ".renamed");
    File renamedMirrorFile = new File(mirrorFile.getAbsolutePath() + ".renamed");

    fs.copy(baseFile, renamedBaseFile);

    assertTrue(baseFile.exists());
    assertTrue(mirrorFile.exists());

    assertTrue(renamedBaseFile.exists());
    assertTrue(renamedMirrorFile.exists());

    assertEquals(baseFile.length(), renamedBaseFile.length());
    assertEquals(mirrorFile.length(), renamedMirrorFile.length());
  }

  @Test
  public void testDelete() throws Exception {
    File baseFile = new File(base, "testFile");
    File mirrorFile = new File(mirror, "testFile");

    assertFalse(baseFile.exists());
    assertFalse(mirrorFile.exists());

    createRandomFile(baseFile);

    assertTrue(baseFile.exists());
    assertTrue(mirrorFile.exists());

    fs.delete(baseFile);

    assertFalse(baseFile.exists());
    assertFalse(mirrorFile.exists());
  }

  @Test
  public void testMkdirs() throws Exception {
    File baseFile = new File(base, "testFile");
    File mirrorFile = new File(mirror, "testFile");

    assertFalse(baseFile.exists());
    assertFalse(mirrorFile.exists());

    fs.mkdirs(baseFile);

    assertTrue(baseFile.exists());
    assertTrue(mirrorFile.exists());
  }

  @Test
  public void testExists() throws Exception {
    File baseFile = new File(base, "testFile");
    File mirrorFile = new File(mirror, "testFile");

    assertFalse(baseFile.exists());
    assertFalse(mirrorFile.exists());

    fs.mkdirs(baseFile);

    assertTrue(fs.exists(baseFile));

    mirrorFile.delete();

    assertFalse(fs.exists(baseFile));
  }

  @Test(expected = AdminException.class)
  public void testError() throws AdminException, IOException {
    mirror.delete();

    File baseFile = new File(base, "testFile");
    File mirrorFile = new File(mirror, "testFile");

    createRandomFile(baseFile);

    mirrorFile.delete();

    try {
      fs.rename(baseFile, new File(baseFile.getAbsolutePath() + ".copy"));
      assertTrue(false);
    } catch (AdminException e) {
    }

    fs.getOutputStream(baseFile, false);
  }
}
