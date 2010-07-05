package ru.novosoft.smsc.util;

import org.junit.Test;

import java.io.*;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class FileUtilsTest {

  private void createRandomFile(File f1) throws IOException {
    OutputStream os = null;
      try {
        os = new FileOutputStream(f1);
        for (int i = 0; i < 100; i++)
          os.write(i);
      } finally {
        if (os != null)
          os.close();
      }
  }

  @Test
  public void copyTest() throws IOException {
    File f1, f2;
    do {
      f1 = new File(System.currentTimeMillis() + ".csv");
      f2 = new File(f1.getAbsolutePath() + ".copy");
    } while (f1.exists() || f2.exists());

    try {
      createRandomFile(f1);

      FileUtils.copyFileTo(f1, f2);

      assertTrue(f2.exists());
      assertEquals(f1.length(), f2.length());
    } finally {
      f1.delete();
      f2.delete();
    }
  }

  @Test
  public void recursivellyDeleteFolderTest() throws IOException {
    File f1;
    do {
      f1 = new File(System.currentTimeMillis() + "");
    } while (f1.exists());

    f1.mkdirs();

    for (int i=0; i<10; i++) {
      File f2 = new File(f1, "subdir" + i);
      f2.mkdir();
      createRandomFile(new File(f2, "test.csv"));
    }

    assertTrue(f1.exists());
    assertFalse(f1.delete());

    FileUtils.recursiveDeleteFolder(f1);

    assertFalse(f1.exists());
  }
}
