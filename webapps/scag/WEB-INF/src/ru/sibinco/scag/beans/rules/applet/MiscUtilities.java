package ru.sibinco.scag.beans.rules.applet;

import java.io.*;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 19.07.2005
 * Time: 15:11:41
 * To change this template use File | Settings | File Templates.
 */
public class MiscUtilities
{
  //{{{ saveBackup() method
  /**
   * Saves a backup (optionally numbered) of a file.
   * @param file A local file
   * @param backups The number of backups. Must be >= 1. If > 1, backup
   * files will be numbered.
   * @param backupPrefix The backup file name prefix
   * @param backupSuffix The backup file name suffix
   * @param backupDirectory The directory where to save backups; if null,
   * they will be saved in the same directory as the file itself.
   * @param backupTimeDistance The minimum time in minutes when a backup
   * version 1 shall be moved into version 2; if 0, backups are always
   * moved.
   * @since jEdit 4.2pre5
   */
  public static void saveBackup(File file, int backups,
    String backupPrefix, String backupSuffix,
    String backupDirectory, int backupTimeDistance)
  {
    if(backupPrefix == null)
      backupPrefix = "";
    if(backupSuffix == null)
      backupSuffix = "";

    String name = file.getName();

    // If backups is 1, create ~ file
    if(backups == 1)
    {
      File backupFile = new File(backupDirectory,
        backupPrefix + name + backupSuffix);
      long modTime = backupFile.lastModified();
      /* if backup file was created less than
       * 'backupTimeDistance' ago, we do not
       * create the backup */
      if(System.currentTimeMillis() - modTime
        >= backupTimeDistance)
      {
        backupFile.delete();
        if (!file.renameTo(backupFile))
          moveFile(file, backupFile);
      }
    }
    // If backups > 1, move old ~n~ files, create ~1~ file
    else
    {
      /* delete a backup created using above method */
       new File(backupDirectory,
        backupPrefix + name + backupSuffix
        + backups + backupSuffix).delete();

      File firstBackup = new File(backupDirectory,
        backupPrefix + name + backupSuffix
        + "1" + backupSuffix);
      long modTime = firstBackup.lastModified();
      /* if backup file was created less than
       * 'backupTimeDistance' ago, we do not
       * create the backup */
      if(System.currentTimeMillis() - modTime
        >= backupTimeDistance)
      {
        for(int i = backups - 1; i > 0; i--)
        {
          File backup = new File(backupDirectory,
            backupPrefix + name
            + backupSuffix + i
            + backupSuffix);

          backup.renameTo(
            new File(backupDirectory,
            backupPrefix + name
            + backupSuffix + (i+1)
            + backupSuffix));
        }

        File backupFile = new File(backupDirectory,
          backupPrefix + name + backupSuffix
          + "1" + backupSuffix);
        if (!file.renameTo(backupFile))
          moveFile(file, backupFile);
      }
    }
  } //}}}

  //{{{ moveFile() method
  /**
   * Moves the source file to the destination.
   *
   * If the destination cannot be created or is a read-only file, the
   * method returns <code>false</code>. Otherwise, the contents of the
   * source are copied to the destination, the source is deleted,
   * and <code>true</code> is returned.
   *
   * @param source The source file to move.
   * @param dest   The destination where to move the file.
   * @return true on success, false otherwise.
   *
   * @since jEdit 4.3pre1
   */
	public static boolean moveFile(File source, File dest)
	{
		boolean ok = false;

		if ((dest.exists() && dest.canWrite())
			|| (!dest.exists() && dest.getParentFile().canWrite()))
		{
			OutputStream fos = null;
			InputStream fis = null;
			try
			{
				fos = new FileOutputStream(dest);
				fis = new FileInputStream(source);
				byte[] buf = new byte[32768];
				int read;
				while ((read = fis.read(buf, 0, buf.length)) != -1)
					fos.write(buf, 0, read);
				ok = true;
			}
			catch (IOException ioe)
			{
			//	Log.log(Log.WARNING, MiscUtilities.class,
				ioe.printStackTrace();//	"Error moving file: " + ioe + " : " + ioe.getMessage());
			}
			finally
			{
				try
				{
					if(fos != null)
						fos.close();
					if(fis != null)
						fis.close();
				}
				catch(Exception e)
				{
					e.printStackTrace();//Log.log(Log.ERROR,MiscUtilities.class,e);
				}
			}

			if(ok)
				source.delete();
		}
		return ok;
	}  //}}}
//{{{ getPermissions() method
  /**
   * Returns numeric permissions of a file. On non-Unix systems, always
   * returns zero.
   * @since jEdit 3.2pre9
   */
  public static int getPermissions(String path)
  {
    int permissions = 0;

   // if(jEditApplet.getBooleanProperty("chmodDisabled"))
   //   return permissions;

    if(OperatingSystem.isUnix())
    {
      String[] cmdarray = { "ls", "-ld", path };

      try
      {
        Process process = Runtime.getRuntime().exec(cmdarray);

        BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));

        String output = reader.readLine();

        if(output != null)
        {
          String s = output.substring(1, 10);

          permissions = MiscUtilities
            .parsePermissions(s);
        }
      }

      // Feb 4 2000 5:30 PM
      // Catch Throwable here rather than Exception.
      // Kaffe's implementation of Runtime.exec throws java.lang.InternalError.
      catch (Throwable t)
      { t.printStackTrace();
      }
    }

    return permissions;
  } //}}}

    public static boolean setPermissions(String path, int permissions)
	{
        boolean result;
				String[] cmdarray = { "chmod", Integer.toString(permissions, 8), path };
				try
				{
					Process process = Runtime.getRuntime().exec(cmdarray);
					process.getInputStream().close();
					process.getOutputStream().close();
					process.getErrorStream().close();
				  result=true;
						// Jun 9 2004 12:40 PM
					// waitFor() hangs on some Java
					// implementations.
					/* int exitCode = process.waitFor();
					if(exitCode != 0)
						Log.log(Log.NOTICE,FileVFS.class,"chmod exited with code " + exitCode); */
				}

				// Feb 4 2000 5:30 PM
				// Catch Throwable here rather than Exception.
				// Kaffe's implementation of Runtime.exec throws java.lang.InternalError.
				catch (Throwable t)
				{ result=false;
				}
     return result;
	} //}}}
//{{{ parsePermissions() method
  /**
   * Parse a Unix-style permission string (rwxrwxrwx).
   * @param s The string (must be 9 characters long).
   * @since jEdit 4.1pre8
   */
  public static int parsePermissions(String s)
  {
    int permissions = 0;

    if(s.length() == 9)
    {
      if(s.charAt(0) == 'r')
        permissions += 0400;
      if(s.charAt(1) == 'w')
        permissions += 0200;
      if(s.charAt(2) == 'x')
        permissions += 0100;
      else if(s.charAt(2) == 's')
        permissions += 04100;
      else if(s.charAt(2) == 'S')
        permissions += 04000;
      if(s.charAt(3) == 'r')
        permissions += 040;
      if(s.charAt(4) == 'w')
        permissions += 020;
      if(s.charAt(5) == 'x')
        permissions += 010;
      else if(s.charAt(5) == 's')
        permissions += 02010;
      else if(s.charAt(5) == 'S')
        permissions += 02000;
      if(s.charAt(6) == 'r')
        permissions += 04;
      if(s.charAt(7) == 'w')
        permissions += 02;
      if(s.charAt(8) == 'x')
        permissions += 01;
      else if(s.charAt(8) == 't')
        permissions += 01001;
      else if(s.charAt(8) == 'T')
        permissions += 01000;
    }

    return permissions;
  } //}}}
  
}
