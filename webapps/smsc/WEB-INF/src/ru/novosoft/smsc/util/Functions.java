package ru.novosoft.smsc.util;

import java.io.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

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
}
