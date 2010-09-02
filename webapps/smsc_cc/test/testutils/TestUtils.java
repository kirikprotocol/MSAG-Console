package testutils;

import junit.framework.AssertionFailedError;

import java.io.*;

/**
 * @author Artem Snopkov
 */
public class TestUtils {

  public static File createRandomFile(String ext) {
    File res;
    do {
      res = new File(System.currentTimeMillis() + ext);
    } while (res.exists());

    return res;
  }

  public static File createRandomDir(String syffix) {
    File dir = createRandomFile(syffix);
    if (dir.mkdirs())
      return dir;
    return null;
  }

  public static void exportResource(InputStream is, File toFile) throws IOException {
    OutputStream os = null;
    try {
      os = new BufferedOutputStream(new FileOutputStream(toFile));

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

  public static void exportResource(InputStream is, File toFile, boolean override) throws IOException {
    if (!override && toFile.exists())
      return;
    exportResource(is, toFile);
  }

  public static File exportResourceToRandomFile(InputStream is, String fileExt) throws IOException {
    File file = createRandomFile(fileExt);
    exportResource(is, file);
    return file;
  }

  public static boolean recursiveDeleteFolder(File folder) {
    File[] childs = folder.listFiles();
    if (childs != null) {
      for (File child : childs) {
        if (child.isDirectory()) {
          if (!recursiveDeleteFolder(child))
            return false;
        } else {
          if (!child.delete())
            return false;
        }
      }
    }
    return folder.delete();
  }
}
