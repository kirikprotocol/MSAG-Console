package ru.novosoft.smsc.util;

import ru.novosoft.smsc.jsp.SMSCAppContext;

import javax.servlet.http.HttpServletRequest;
import java.io.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.*;

/**
 * Created by igork
 * Date: Jan 20, 2003
 * Time: 6:41:15 PM
 */
public class Functions
{
	public static boolean recursiveDeleteFolder(File folder)
	{
		String[] childNames = folder.list();
		if (childNames != null)
		{
			for (int i = 0; i < childNames.length; i++)
			{
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
		for (ZipEntry e = zin.getNextEntry(); e != null; e = zin.getNextEntry())
		{
			if (!e.isDirectory())
				unZipFileFromArchive(folderUnpackTo, e.getName(), zin);
		}
		zin.close();
		in.close();
	}

	public static File saveFileToTemp(InputStream in,
												 String prefix, //"SMSC_SME_distrib_"
												 String sufffix //".zip.tmp"
												 )
			throws IOException
	{
		File tmpFile = File.createTempFile(prefix, sufffix);
		OutputStream out = new BufferedOutputStream(new FileOutputStream(tmpFile));

		byte buffer[] = new byte[2048];
		for (int readed = 0; (readed = in.read(buffer)) > -1;)
		{
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
		out.println("<?xml version=\"1.0\" encoding=\"" + (encoding == null || encoding.length() == 0 ? "ISO-8859-1" : encoding) + "\"?>");
		out.println("<!DOCTYPE " + docType + " SYSTEM \"file://" + dtdFile + "\">");
		out.println();
		out.println("<" + docType + ">");
		return out;
	}

	public static PrintWriter storeConfigFooter(PrintWriter out, String docType)
	{
		out.println("</" + docType + ">");
		return out;
	}

	public static SMSCAppContext getAppContext(HttpServletRequest request)
	{
		return (SMSCAppContext) request.getAttribute("appContext");
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
   * @param checkedTasksSet values will be added to this collection
   * @param values string to tokenize
   * @param delimeter string values delimiter
   * @param trimValues if true, each value will be trimmed before add to collection
   */
  public static void addValuesToCollection(Collection checkedTasksSet, final String values, final String delimeter, boolean trimValues)
  {
    for (StringTokenizer i = new StringTokenizer(values, delimeter, false); i.hasMoreTokens();) {
      checkedTasksSet.add(trimValues ? i.nextToken().trim() : i.nextToken());
    }
  }

  private static int filenameCounter = 0;
  public static synchronized File createTempFilename(String prefix, String suffix, File directory)
  {
    File file = new File(directory, prefix + (filenameCounter++));
    while(file.exists())
      file = new File(directory, prefix + (filenameCounter++));
    return file;
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
}
