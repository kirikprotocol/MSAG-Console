package ru.sibinco.lib.backend.util;

import org.apache.log4j.Logger;
import ru.sibinco.lib.SibincoException;

import java.io.*;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


/**
 * Created by igork
 * Date: Jan 20, 2003
 * Time: 6:41:15 PM
 */
public class Functions
{
  private static Logger logger = Logger.getLogger(Functions.class.getName());
  private static final String backup_dir_name = "backup";

  public static boolean recursiveDeleteFolder(File folder)
  {
    String[] childNames = folder.list();
    if (childNames != null) {
      for (int i = 0; i < childNames.length; i++) {
        File child = new File(folder, childNames[i]);
        if (child.isDirectory())
          recursiveDeleteFolder(child);
        else
          child.delete();
      }
    }
    return folder.delete();
  }

  public static void unZipFileFromArchive(File folderUnpackTo, String name, ZipInputStream zin)
      throws IOException
  {
    File file = new File(folderUnpackTo, name);
    file.getParentFile().mkdirs();
    OutputStream out = new BufferedOutputStream(new FileOutputStream(file));
    for (int i = 0; (i = zin.read()) != -1; out.write(i)) ;
    out.close();
  }

  public static void unZipArchive(File folderUnpackTo, InputStream in)
      throws IOException
  {
    ZipInputStream zin = new ZipInputStream(in);
    for (ZipEntry e = zin.getNextEntry(); e != null; e = zin.getNextEntry()) {
      if (!e.isDirectory())
        unZipFileFromArchive(folderUnpackTo, e.getName(), zin);
    }
    zin.close();
    in.close();
  }

  public static File saveFileToTemp(InputStream in, File file)
      throws IOException
  {
    File tmpFile = Functions.createNewFilenameForSave(file);
    OutputStream out = new BufferedOutputStream(new FileOutputStream(tmpFile));

    byte buffer[] = new byte[2048];
    for (int readed = 0; (readed = in.read(buffer)) > -1;) {
      out.write(buffer, 0, readed);
    }

    in.close();
    out.close();
    return tmpFile;
  }

  public static PrintWriter storeConfigHeader(PrintWriter out, String docType, String dtdFile)
  {
    // C++ code doesn't know about other codings // System.getProperty("file.encoding");
    return storeConfigHeader(out, docType, dtdFile, null);
  }

  public static PrintWriter storeConfigHeader(PrintWriter out, String docType, String dtdFile, String encoding)
  {
    out.println("<?xml version=\"1.0\" encoding=\"" + (encoding == null || encoding.length() == 0 ? Functions.getLocaleEncoding() : encoding) + "\"?>");
    out.println("<!DOCTYPE " + docType + " SYSTEM \"" + dtdFile + "\">");
    out.println();
    out.println("<" + docType + ">");
    return out;
  }

  public static PrintWriter storeConfigFooter(PrintWriter out, String docType)
  {
    out.println("</" + docType + ">");
    return out;
  }

  public static String collectionToString(final Collection collection, final String delimeter)
  {
    StringBuffer result = new StringBuffer();
    for (Iterator i = collection.iterator(); i.hasNext();) {
      result.append(i.next().toString());
      if (i.hasNext())
        result.append(delimeter);
    }
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
  public static void addValuesToCollection(Collection checkedTasksSet, final String values, final String delimeter, boolean trimValues)
  {
    for (StringTokenizer i = new StringTokenizer(values, delimeter, false); i.hasMoreTokens();) {
      checkedTasksSet.add(trimValues ? i.nextToken().trim() : i.nextToken());
    }
  }

  private static long filenameCounter = 0;

  public static synchronized File createTempFilename(String prefix, String suffix, File directory)
  {
    File file = new File(directory, prefix + suffix);
    while (file.exists()) {
      file = new File(directory, prefix + (filenameCounter++) + suffix);
      if (filenameCounter == Long.MAX_VALUE) filenameCounter = 0;
    }
    return file;
  }

  private static final DateFormat suffixDateFormat = new SimpleDateFormat(".yyyy.MM.dd.HH.mm.ss.SSS.'bak'");

  public static synchronized File createNewFilenameForSave(final File filenameToSave)
  {
    final String suffix = suffixDateFormat.format(new Date());
    final File directory = filenameToSave.getParentFile();
    final String filename = filenameToSave.getName() + ".new";

    // create temp file for new config file
    File newFilename = new File(directory, filename);
    if (newFilename.exists())
      newFilename = Functions.createTempFilename(filename, suffix, directory);

    return newFilename;
  }

  public static final void renameNewSavedFileToOriginal(File newCreatedFile, File oldFileRenameTo) throws IOException
  {
    final String suffix = suffixDateFormat.format(new Date());

    // rename old config file to bakup file
    String oldFilename = oldFileRenameTo.getAbsolutePath();
    if (oldFileRenameTo.exists()) {
      File backupDir = new File(oldFileRenameTo.getParentFile(), backup_dir_name);
      if (!backupDir.exists()) {
        if (!backupDir.mkdirs()) {
          logger.warn("Could not create backup directory \"" + backupDir.getAbsolutePath() + "\", using \"" + oldFileRenameTo.getParentFile().getAbsolutePath() + "\"");
          backupDir = oldFileRenameTo.getParentFile();
        }
      }
      final File backFile = Functions.createTempFilename(oldFileRenameTo.getName(), suffix, backupDir);
      if (!new File(oldFilename).renameTo(backFile)) {
        logger.error("Couldn't rename old file \"" + oldFilename + "\" to backup file \"" + backFile.getAbsolutePath() + '"');
        throw new IOException("Couldn't rename old file \"" + oldFilename + "\" to backup file \"" + backFile.getAbsolutePath() + '"');
      }
    }

    //rename temp new file to desired file
    final String newFilename = newCreatedFile.getAbsolutePath();
    if (!new File(newFilename).renameTo(oldFileRenameTo)) {
      logger.error("Couldn't rename new file \"" + newFilename + "\" to old file \"" + oldFilename + '"');
      ;
      throw new IOException("Couldn't rename new file \"" + newFilename + "\" to old file \"" + oldFilename + '"');
    }
  }

  public static final void SavedFileToBackup(File newCreatedFile, String suffixToDelete) throws IOException
  {
    final String suffix = suffixDateFormat.format(new Date());

    // rename old config file to bakup file
    String newCreated = newCreatedFile.getAbsolutePath();
    if (newCreatedFile.exists()) {
      File backupDir = new File(newCreatedFile.getParentFile(), backup_dir_name);
      if (!backupDir.exists()) {
        if (!backupDir.mkdirs()) {
          logger.warn("Could not create backup directory \"" + backupDir.getAbsolutePath() + "\", using \"" + newCreatedFile.getParentFile().getAbsolutePath() + "\"");
          backupDir = newCreatedFile.getParentFile();
        }
      }
      final File backFile = Functions.createTempFilename(newCreatedFile.getName().substring(0,newCreatedFile.getName().length()-suffixToDelete.length()), suffix, backupDir);
      if (!new File(newCreated).renameTo(backFile)) {
        logger.error("Couldn't rename old file \"" + newCreated + "\" to backup file \"" + backFile.getAbsolutePath() + '"');
        throw new IOException("Couldn't rename old file \"" + newCreated + "\" to backup file \"" + backFile.getAbsolutePath() + '"');
      }
    }
  }

  public static Date truncateTime(Date dateTime)
  {
    Calendar calendar = new GregorianCalendar();
    calendar.setTime(dateTime);
    calendar.set(Calendar.HOUR_OF_DAY, 0);
    calendar.set(Calendar.MINUTE, 0);
    calendar.set(Calendar.SECOND, 0);
    calendar.set(Calendar.MILLISECOND, 0);
    return calendar.getTime();
  }

  public static String getServiceId(String servletPath) throws SibincoException
  {
    final String prefix = "/esme_";
    if (servletPath == null || !servletPath.startsWith(prefix)) {
      logger.error("Call getServiceId(String servletPath) with incorrect parameter \"" + servletPath + "\"");
      throw new SibincoException("Call getServiceId(String servletPath) with incorrect parameter \"" + servletPath + "\"");
    }
    final int secondSlashIndex = servletPath.indexOf('/', prefix.length());
    if (secondSlashIndex < prefix.length()) {
      logger.error("Call getServiceId(String servletPath) with incorrect parameter \"" + servletPath + "\"");
      throw new SibincoException("Call getServiceId(String servletPath) with incorrect parameter \"" + servletPath + "\"");
    }
    return servletPath.substring(prefix.length(), secondSlashIndex);
  }

  public static String[] trimStrings(String[] masks)
  {
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

  public static String getLocaleEncoding()
  {
    System.out.println("Webapp encoding: " + System.getProperty("file.encoding", "WINDOWS-1251"));
    return System.getProperty("file.encoding", "WINDOWS-1251");
  }
  
  public static void readBuffer( InputStream is, byte buffer[], int size ) throws IOException {
    int read = 0;
    while (read < size) {
      int result = is.read(buffer, read, size - read);
      if (result < 0) throw new EOFException("Failed to read " + size + " bytes, read failed at "+read);
      read += result;
    }
  }
}
