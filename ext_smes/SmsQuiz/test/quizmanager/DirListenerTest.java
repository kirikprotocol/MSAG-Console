package quizmanager;

import org.junit.*;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertFalse;
import mobi.eyeline.smsquiz.quizmanager.dirlistener.DirListener;

import java.io.*;

/**
 * author: alkhal
 */
public class DirListenerTest {
  private DirListener dirListener;

  @Before
  public void init() {
    try {
      dirListener = new DirListener("test_QuizManager");
      File file = new File("test_QuizManager");
      if (file.exists()) {
        removeAll(file);
      }
      file.mkdirs();
    } catch (Exception e) {
      e.printStackTrace();
      assertFalse(true);
    }
  }

  @Test
  public void testInit() {
    assertTrue(dirListener.countFiles() == 0);
  }

  @Test
  public void addFile() {
    FileWriter writer = null;
    try {
      writer = new FileWriter("test_QuizManager/test_opros.xml");
    } catch (IOException e) {
      e.printStackTrace();
      assertTrue(false);
    } finally {
      try {
        if (writer != null) {
          writer.close();
        }
      } catch (IOException e) {
        e.printStackTrace();
        assertTrue(false);
      }
    }
    dirListener.run();
    assertTrue(dirListener.countFiles() == 1);
  }

  @Test
  public void changeFile() {
    FileWriter writer = null;
    try {
      writer = new FileWriter("test_QuizManager/test_opros.xml");
    } catch (IOException e) {
      e.printStackTrace();
      assertTrue(false);
    } finally {
      try {
        if (writer != null) {
          writer.close();
        }
      } catch (IOException e) {
        e.printStackTrace();
        assertTrue(false);
      }
    }
    dirListener.run();
    assertTrue(dirListener.countFiles() == 1);
  }

  private void removeAll(File dir) {
    if ((dir == null) || (!dir.exists())) {
      return;
    }
    if (dir.isDirectory()) {
      for (File f : dir.listFiles()) {
        removeAll(f);
      }
    }
    if (dir.isFile()) {
      dir.delete();
    }
  }

}
