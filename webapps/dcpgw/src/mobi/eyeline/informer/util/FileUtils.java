package mobi.eyeline.informer.util;

import java.io.*;
import java.nio.channels.FileChannel;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * Утилиты для работы с файлами.
 *
 * @author Artem Snopkov
 */
public class FileUtils {

  /**
   * Copies one file to another
   *
   * @param source      source file
   * @param destination destination file
   * @return кол-во скопированных байт
   * @throws java.io.IOException  if I/O problems occured
   * @throws NullPointerException if source or destination is null
   */
  public static long copyFileTo(File source, File destination) throws IOException {
    FileInputStream src = null;
    FileOutputStream dest = null;
    try {
      src = new FileInputStream(source);
      dest = new FileOutputStream(destination);

      FileChannel srcChannel = src.getChannel();
      FileChannel destChannel = dest.getChannel();

      return srcChannel.transferTo(0, srcChannel.size(), destChannel);
    } finally {
      if (src != null) {
        try {
          src.close();
        } catch (IOException ignored) {
        }
      }
      if (dest != null) {
        try {
          dest.close();
        } catch (IOException ignored) {
        }
      }
    }
  }

  /**
   * Deletes folder and all it's subfolders
   *
   * @param folder folder to delete
   * @return true, if folder was successfully removed
   * @throws NullPointerException if source or destination is null
   */
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
