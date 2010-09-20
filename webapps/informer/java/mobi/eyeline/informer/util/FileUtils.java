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
   * @throws java.io.IOException          if I/O problems occured
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

  /**
   * Unzips files from zip input stream to specified folder
   *
   * @param folderUnpackTo parent folder name
   * @param name           destination folder name
   * @param zin            zip input stream
   * @throws IOException if any I/O problems occured
   */
  public static void unZipFileFromArchive(File folderUnpackTo, String name, ZipInputStream zin) throws IOException {
    File file = new File(folderUnpackTo, name);
    file.getParentFile().mkdirs();
    OutputStream out = null;
    try {
      out = new BufferedOutputStream(new FileOutputStream(file));
      int i;
      while ((i = zin.read()) != -1)
        out.write(i);
    } finally {
      if (out != null)
        out.close();
    }
  }

  /**
   * Unzips archive from input stream to specified folder
   *
   * @param folderUnpackTo destination folder
   * @param in             input stream with zip archive
   * @throws IOException if any I/O problems occured
   */
  public static void unZipArchive(File folderUnpackTo, InputStream in) throws IOException {
    ZipInputStream zin = null;
    try {
      zin = new ZipInputStream(in);
      for (ZipEntry e = zin.getNextEntry(); e != null; e = zin.getNextEntry()) {
        if (!e.isDirectory())
          unZipFileFromArchive(folderUnpackTo, e.getName(), zin);
      }
    } finally {
      if (zin != null)
        zin.close();
    }
  }
}
