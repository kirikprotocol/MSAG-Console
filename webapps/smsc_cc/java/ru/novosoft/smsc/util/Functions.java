package ru.novosoft.smsc.util;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.Constants;

import java.io.*;
import java.nio.channels.FileChannel;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


/**
 * Created by artem
 */
public class Functions {

  private static Category logger = Category.getInstance(Functions.class);
  private static final String backup_dir_name = "backup";

  /**
   * Copies one file to another
   *
   * @param source      source file
   * @param destination destination file
   * @throws IOException          if I/O problems occured
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

  /**
   * Creates a temporary copy of file. Copy file name contains timestamp.
   *
   * @param in input stream contains source file
   * @param file file name base
   * @return created temporary file
   * @throws IOException if any I/O problems occured
   */
  public static File saveFileToTemp(InputStream in, File file) throws IOException {
    File tmpFile = Functions.createNewFilenameForSave(file);

    OutputStream out = null;
    try {
      out = new BufferedOutputStream(new FileOutputStream(tmpFile));

      byte buffer[] = new byte[2048];
      int readed;
      while ((readed = in.read(buffer)) > -1)
        out.write(buffer, 0, readed);

    } finally {
      if (out != null) {
        try {
          out.close();
        } catch (IOException ignored) {}
      }
      in.close();
    }

    return tmpFile;
  }

//  public static PrintWriter storeConfigHeader(PrintWriter out, String docType, String dtdFile) {
//    // C++ code doesn't know about other codings // System.getProperty("file.encoding");
//    return storeConfigHeader(out, docType, dtdFile, null);
//  }
//
//  public static PrintWriter storeConfigHeader(PrintWriter out, String docType, String dtdFile, String encoding) {
//    out.println("<?xml version=\"1.0\" encoding=\"" + (encoding == null || encoding.length() == 0 ? Functions.getLocaleEncoding() : encoding) + "\"?>");
//    out.println("<!DOCTYPE " + docType + " SYSTEM \"" + dtdFile + "\">");
//    out.println();
//    out.println("<" + docType + ">");
//    return out;
//  }
//
//  public static PrintWriter storeConfigFooter(PrintWriter out, String docType) {
//    out.println("</" + docType + ">");
//    return out;
//  }

//    public static SMSCAppContext getAppContext(HttpServletRequest request) {
//        return (SMSCAppContext) request.getAttribute("appContext");
//    }

  /**
   * Returns string contains all emenetns of collection with specified delimeter
   *
   * @param collection collection
   * @param delimeter delimeter
   * @return string
   */
  public static String collectionToString(final Collection collection, final String delimeter) {
    StringBuilder result = new StringBuilder(10*collection.size());
    for (Iterator i = collection.iterator(); i.hasNext();) {
      result.append(i.next().toString());
      if (i.hasNext())
        result.append(delimeter);
    }
    return result.toString();
  }

  /**
   * Returns string contains all emenetns of array with specified delimeter
   *
   * @param array array
   * @param delimeter delimeter
   * @return string
   */
  public static String arrayToString(final Object[] array, final String delimeter) {
    if(array.length == 0) {
      return "";
    }
    StringBuilder result = new StringBuilder(10*array.length);
    for (int i = 0; i < array.length-1; i++) {
      result.append(array[i].toString()).append(delimeter);
    }
    result.append(array[array.length-1].toString());

    return result.toString();
  }

  /**
   * Parses string to values by StringTokenizer and adds values to collection in order returned by StringTokenizer.
   *
   * @param checkedTasksSet values will be added to this collection
   * @param values          string to tokenize
   * @param delimeter       string values delimiter
   * @param trimValues      if true, each value will be trimmed before add to collection
   */
  public static void addValuesToCollection(Collection checkedTasksSet, final String values, final String delimeter, boolean trimValues) {
    for (StringTokenizer i = new StringTokenizer(values, delimeter, false); i.hasMoreTokens();) {
      checkedTasksSet.add(trimValues ? i.nextToken().trim() : i.nextToken());
    }
  }

  private static long filenameCounter = 0;

  /**
   * Creates temp file name
   * @param prefix name prefix
   * @param suffix name suffix
   * @param directory temp file directory
   * @return temp file instance
   */
  public static synchronized File createTempFilename(String prefix, String suffix, File directory) {
    File file = new File(directory, prefix + suffix);
    while (file.exists()) {
      file = new File(directory, prefix + (filenameCounter++) + suffix);
      if (filenameCounter == Long.MAX_VALUE) filenameCounter = 0;
    }
    return file;
  }

  private static final DateFormat suffixDateFormat = new SimpleDateFormat(".yyyy.MM.dd.HH.mm.ss.SSS.'bak'");

  /**
   * Adds timestamp to file name
   * @param filenameToSave original file name
   * @return new file name
   */
  public static synchronized File createNewFilenameForSave(final File filenameToSave) {
    final String suffix = suffixDateFormat.format(new Date());
    final File directory = filenameToSave.getParentFile();
    final String filename = filenameToSave.getName() + ".new";

    // create temp file for new config file
    File newFilename = new File(directory, filename);
    if (newFilename.exists())
      newFilename = Functions.createTempFilename(filename, suffix, directory);

    return newFilename;
  }

  public static final void renameNewSavedFileToOriginal(File newCreatedFile, File oldFileRenameTo) throws IOException {
    final String suffix = suffixDateFormat.format(new Date());

    // rename old config file to bakup file
    String oldFilename = oldFileRenameTo.getAbsolutePath();
    if (oldFileRenameTo.exists()) {
      File backupDir = new File(oldFileRenameTo.getParentFile(), backup_dir_name);
      if (!backupDir.exists()) {
        if (!backupDir.mkdirs()) {
          logger.error("Could not create backup directory \"" + backupDir.getAbsolutePath() + "\", using \"" + oldFileRenameTo.getParentFile().getAbsolutePath() + "\"");
          backupDir = oldFileRenameTo.getParentFile();
        }
      }
      final File backFile = Functions.createTempFilename(oldFileRenameTo.getName(), suffix, backupDir);
      if (!new File(oldFilename).renameTo(backFile))
        throw new IOException("Couldn't rename old file \"" + oldFilename + "\" to backup file \"" + backFile.getAbsolutePath() + '"');
    }

    final String newFilename = newCreatedFile.getAbsolutePath();
    if (Constants.isMirrorNeeded()) {
      try {
        copyFileTo(newCreatedFile, Constants.getMirrorFile(oldFileRenameTo));
      } catch (IOException e) {
        e.printStackTrace();
        Constants.setMirrorSaveErrorAppeared(true);
      }
    }
    //rename temp new file to desired file
    if (!new File(newFilename).renameTo(oldFileRenameTo))
      throw new IOException("Couldn't rename new file \"" + newFilename + "\" to old file \"" + oldFilename + '"');
  }

  /**
   * Set hour, minute, seconds and milliseconds to 0
   * @param dateTime ooriginal date
   * @return new date
   */
  public static Date truncateTime(Date dateTime) {
    Calendar calendar = new GregorianCalendar();
    calendar.setTime(dateTime);
    calendar.set(Calendar.HOUR_OF_DAY, 0);
    calendar.set(Calendar.MINUTE, 0);
    calendar.set(Calendar.SECOND, 0);
    calendar.set(Calendar.MILLISECOND, 0);
    return calendar.getTime();
  }

//    public static String getServiceId(String servletPath) throws AdminException {
//        final String prefix = "/esme_";
//        if (servletPath == null || !servletPath.startsWith(prefix)) {
//            logger.error("Call getServiceId(String servletPath) with incorrect parameter \"" + servletPath + "\"");
//            throw new AdminException("Call getServiceId(String servletPath) with incorrect parameter \"" + servletPath + "\"");
//        }
//        final int secondSlashIndex = servletPath.indexOf('/', prefix.length());
//        if (secondSlashIndex < prefix.length()) {
//            logger.error("Call getServiceId(String servletPath) with incorrect parameter \"" + servletPath + "\"");
//            throw new AdminException("Call getServiceId(String servletPath) with incorrect parameter \"" + servletPath + "\"");
//        }
//        return servletPath.substring(prefix.length(), secondSlashIndex);
//    }

  /**
   * 1. Trim all strings in specified array
   * 2. Removes strings that contains only spaces
   * 3. Removes null strings
   * 4. Removes duplicates
   * 5. Sort array
   *
   * @param masks array
   * @return new array
   * @deprecated too complex function!!!
   */
  public static String[] trimStrings(String[] masks) {
    Set newMasks = new HashSet(masks.length);
    for (int i = 0; i < masks.length; i++) {
      String mask = masks[i];
      if (mask != null) {
        final String m = mask.trim();
        if (m.length() > 0)
          newMasks.add(m);
      }
    }
    String result[] = (String[]) newMasks.toArray(new String[0]);
    Arrays.sort(result);
    return result;
  }

  /**
   * 1. Trim all strings in specified collection
   * 2. Removes strings that contains only spaces
   * 3. Removes null strings
   *
   * @param strings array
   * @return new array
   * @deprecated too complex function!!!
   */
  public static List trimStrings(Collection strings) {
    List result = new LinkedList();
    if (strings != null) {
      for (Iterator i = strings.iterator(); i.hasNext();) {
        String str = (String) i.next();
        if (str != null && str.trim().length() > 0)
          result.add(str.trim());
      }
    }
    return result;
  }

  /**
   * Returns file.encoding
   * @return file.encoding
   */
  public static String getLocaleEncoding() {
    return System.getProperty("file.encoding", "WINDOWS-1251");
  }

  /**
   * Compares 2 strings
   * @param s1 first string
   * @param s2 second string
   * @return true if first string is equals to second string
   * @deprecated bad function name
   */
  public static boolean compareStrs(String s1, String s2) {
    return (s1 == null && s2 == null) || (s1 != null && s2 != null && s1.equals(s2));
  }

//  public static void readBuffer(InputStream is, byte buffer[], int size) throws IOException {
//    int read = 0;
//    while (read < size) {
//      int result = is.read(buffer, read, size - read);
//      if (result < 0) throw new EOFException("Failed to read " + size + " bytes, read failed at " + read);
//      read += result;
//    }
//  }

  /**
   * Converts time from one timezone to another
   * @param time time
   * @param fromTimezone source timezone
   * @param toTimezone destinatin timezone
   * @return converted time
   * @throws NullPointerException if any of arguments is null
   */
  public static Date convertTime(final Date time, TimeZone fromTimezone, TimeZone toTimezone) {
    // Find time difference
    final long d1 = fromTimezone.getOffset(time.getTime());
    final long d2 = toTimezone.getOffset(time.getTime());
    
    Calendar calend = Calendar.getInstance();
    calend.setTimeInMillis(time.getTime() + d2 - d1);
    return calend.getTime();
  }
}
