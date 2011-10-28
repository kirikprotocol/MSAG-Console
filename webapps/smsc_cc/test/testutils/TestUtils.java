package testutils;

import junit.framework.AssertionFailedError;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.*;

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

  public static void exportResource(InputStream is, File toFile, FileSystem fs) throws IOException {
    OutputStream os = null;
    try {
      os = new BufferedOutputStream(fs.getOutputStream(toFile));

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

  public static void exportResource(InputStream is, File toFile, boolean override, FileSystem fs) throws IOException, AdminException {
    if (!override && fs.exists(toFile))
      return;
    exportResource(is, toFile, fs);
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
