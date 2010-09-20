/*
 * FileVFS.java - Local filesystem VFS
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 1998, 2005 Slava Pestov
 * Portions copyright (C) 1998, 1999, 2000 Peter Graves
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

package org.gjt.sp.jedit.io;

//{{{ Imports
import javax.swing.filechooser.FileSystemView;
import java.awt.Component;
import java.io.*;
import java.text.*;
import java.util.Date;
import java.util.HashMap;

import org.gjt.sp.jedit.*;
import org.gjt.sp.util.Log;
//}}}

/**
 * Local filesystem VFS.
 * @author Slava Pestov
 * @version $Id$
 */
public class FileVFS extends VFS
{
 public static final String PERMISSIONS_PROPERTY = "FileVFS__perms";

/* //{{{ FileVFS method
 public FileVFS()
 {
  super("file",READ_CAP | WRITE_CAP | DELETE_CAP
   | RENAME_CAP | MKDIR_CAP | LOW_LATENCY_CAP
   | CASE_INSENSITIVE_CAP ,
   new String[] { EA_TYPE, EA_SIZE, EA_STATUS,
   EA_MODIFIED });
 } //}}}
  */
  //{{{ FileVFS method
  public FileVFS()
  {
    super("file",READ_CAP | WRITE_CAP | DELETE_CAP
      | RENAME_CAP | MKDIR_CAP | LOW_LATENCY_CAP
      | ((OperatingSystem.isCaseInsensitiveFS())
      ? CASE_INSENSITIVE_CAP : 0),
      new String[] { EA_TYPE, EA_SIZE, EA_STATUS,
      EA_MODIFIED });
  } //}}}

 //{{{ getParentOfPath() method
 public String getParentOfPath(String path)
 {
  if(OperatingSystem.isDOSDerived())
  {
   if(path.length() == 2 && path.charAt(1) == ':')
    return FileRootsVFS.PROTOCOL + ":";
   else if(path.length() == 3 && path.endsWith(":\\"))
    return FileRootsVFS.PROTOCOL + ":";
   else if(path.startsWith("\\\\") && path.indexOf('\\',2) == -1)
    return path;
  }

  return super.getParentOfPath(path);
 } //}}}

 //{{{ constructPath() method
 public String constructPath(String parent, String path)
 {
  if(parent.endsWith(""+jEdit.separatorChar)
   || parent.endsWith("/"))
   return parent + path;
  else
   return parent + ""+jEdit.separatorChar + path;
 } //}}}

 //{{{ getFileSeparator() method
 public char getFileSeparator()
 {
  return jEdit.separatorChar;
 } //}}}

 //{{{ getTwoStageSaveName() method
 /**
  * Returns a temporary file name based on the given path.
  *
  * <p>If the directory where the file would be created cannot be
  * written (i.e., no new files can be created in that directory),
  * this method returns <code>null</code>.</p>
  *
  * @param path The path name
  */
 public String getTwoStageSaveName(String path)
 {
  File parent = new File(getParentOfPath(path));
  return (jEdit.BoolGet(parent.getPath(),jEdit.getCanWrite()))
   ? super.getTwoStageSaveName(path)
   : null;
 } //}}}

 //{{{ save() method
 public boolean save(View view, Buffer buffer, String path)
 {
  if(OperatingSystem.isUnix())
  {
   int permissions = getPermissions(buffer.getPath());
  // Log.log(Log.DEBUG,this,buffer.getPath() + " has permissions 0"
  //   + Integer.toString(permissions,8));
   buffer.setIntegerProperty(PERMISSIONS_PROPERTY,permissions);
  }

  return super.save(view,buffer,path);
 } //}}}

 //{{{ insert() method
 public boolean insert(View view, Buffer buffer, String path)
 {
  File file = new File(path);

  //{{{ Check if file is valid
  if(!jEdit.BoolGet(path,jEdit.getExists())) //file.exists())
   return false;

  if(jEdit.BoolGet(path,jEdit.getIsDirectory()))//file.isDirectory())
  {
   VFSManager.error(view,path,
    "ioerror.open-directory",null);
   return false;
  }

  if(!jEdit.BoolGet(path,jEdit.getCanRead()))//file.canRead())
  {
   VFSManager.error(view,path,
    "ioerror.no-read",null);
   return false;
  } //}}}

  return super.insert(view,buffer,path);
 } //}}}

 //{{{ _canonPath() method
 /**
  * Returns the canonical form if the specified path name. For example,
  * <code>~</code> might be expanded to the user's home directory.
  * @param session The session
  * @param path The path
  * @param comp The component that will parent error dialog boxes
  * @exception IOException if an I/O error occurred
  * @since jEdit 4.0pre2
  */
 public String _canonPath(Object session, String path, Component comp)
  throws IOException
 {
  return MiscUtilities.canonPath(path);
 } //}}}

 //{{{ LocalFile class
 public static class LocalFile extends VFSFile
 {
  private File file;
    private String filePath;

    // use system default short format
  public static DateFormat DATE_FORMAT
   = DateFormat.getInstance();

  /**
   * @deprecated Call getModified() instead.
   */
  public long modified;



  public LocalFile(File file)
  {
   this.file = file;

   /* These attributes are fetched relatively
   quickly. The rest are lazily filled in. */
   setName(file.getName());
   String path = file.getPath();
     if (File.separatorChar!=jEdit.separatorChar)
        path= path.replace(File.separatorChar,jEdit.separatorChar);
      this.filePath=path;
      setPath(path);
   setDeletePath(path);
   setHidden(jEdit.BoolGet(path,jEdit.getIsHidden()));//file.isHidden());
   setType(jEdit.BoolGet(path,jEdit.getIsDirectory())  //file.isDirectory()
    ? VFSFile.DIRECTORY
    : VFSFile.FILE);
  }

  public String getExtendedAttribute(String name)
  {
   if(name.equals(EA_MODIFIED))
    return DATE_FORMAT.format(new Date(modified));
   else
    return super.getExtendedAttribute(name);
  }
  
  protected void fetchAttrs()
  {
   if(fetchedAttrs())
    return;

   super.fetchAttrs();

   setSymlinkPath(MiscUtilities.resolveSymlinks(
    filePath));
   setReadable(jEdit.BoolGet(filePath,jEdit.getCanRead()));//file.canRead());
   setWriteable(jEdit.BoolGet(filePath,jEdit.getCanWrite()));//file.canWrite());
   setLength(jEdit.LongGet(filePath,jEdit.getLength()));//file.length());
   setModified(jEdit.LongGet(filePath,jEdit.getLastModified()));//file.lastModified());
  }
  
  public String getSymlinkPath()
  {
   fetchAttrs();
   return super.getSymlinkPath();
  }
  
  public long getLength()
  {
   fetchAttrs();
   return super.getLength();
  }
  
  public boolean isReadable()
  {
   fetchAttrs();
   return super.isReadable();
  }
  
  public boolean isWriteable()
  {
   fetchAttrs();
   return super.isWriteable();
  }

  public long getModified()
  {
   return modified;
  }

  public void setModified(long modified)
  {
   this.modified = modified;
  }
 } //}}}

 //{{{ _listFiles() method
 public VFSFile[] _listFiles(Object session, String path,
  Component comp)
 {
  //{{{ Windows work around
  /* On Windows, paths of the form X: list the last *working
   * directory* on that drive. To list the root of the drive,
   * you must use X:\.
   *
   * However, the VFS browser and friends strip off trailing
   * path separators, for various reasons. So to work around
   * that, we add a '\' to drive letter paths on Windows.
   */
  if(OperatingSystem.isWindows())
  {
   if(path.length() == 2 && path.charAt(1) == ':')
    path = path.concat(""+jEdit.separatorChar);
  } //}}}

  File directory = new File(path);
    File[] list = jEdit.FilesGet(path,jEdit.getNotHiddenFiles());//File[] list = fsView.getFiles(directory,false);
  if(list == null)
  {
   VFSManager.error(comp,path,"ioerror.directory-error-nomsg",null);
   return null;
  }

  VFSFile[] list2 = new VFSFile[list.length];
  for(int i = 0; i < list.length; i++) {
   list2[i] = new LocalFile(list[i]);
    }
  return list2;
 } //}}}

 //{{{ _getFile() method
 public VFSFile _getFile(Object session, String path,
  Component comp)
 {
  if(path.equals("/") && OperatingSystem.isUnix())
  {
   return new VFS.DirectoryEntry(path,path,path,
    VFSFile.DIRECTORY,0L,false);
  }

  File file = new File(path);
  if(!jEdit.BoolGet(path,jEdit.getExists()))
   return null;

  return new LocalFile(file);
 } //}}}

 //{{{ _delete() method
 public boolean _delete(Object session, String path, Component comp)
 {
  File file = new File(path);
  // do some platforms throw exceptions if the file does not exist
  // when we ask for the canonical path?
  String canonPath;
 // try
 //{
   canonPath = jEdit.StringGet(path,jEdit.getCanonPath());//file.getCanonicalPath();
 // }
//  catch(IOException io)
//  {
 //  canonPath = path;
 // }

  boolean retVal = jEdit.BoolGet(path,jEdit.getDelete());//file.delete();
  if(retVal)
   VFSManager.sendVFSUpdate(this,canonPath,true);
  return retVal;
 } //}}}

 //{{{ _rename() method
 public boolean _rename(Object session, String from, String to,
  Component comp)
 {
  File _to = new File(to);

  String toCanonPath;
//  try
//  {
   toCanonPath =jEdit.StringGet(to,jEdit.getCanonPath());// _to.getCanonicalPath();
//  }
//  catch(IOException io)
//  {
//   toCanonPath = to;
//  }

  // this is needed because on OS X renaming to a non-existent
  // directory causes problems
  File parent = new File(_to.getParent());
  if(jEdit.BoolGet(parent.getPath(),jEdit.getExists()))//parent.exists())
  {
   if(!jEdit.BoolGet(parent.getPath(),jEdit.getIsDirectory()))//parent.isDirectory())
        return false;
  }
  else
  {
   jEdit.BoolGet(parent.getPath(),jEdit.getMkDirs());//parent.mkdirs();

   if(!jEdit.BoolGet(parent.getPath(),jEdit.getExists()))//parent.exists())
    return false;
  }
   System.out.println("FileVFS _rename line 366 from(savePath)= "+from+" to= "+to);
  File _from = new File(from);

  String fromCanonPath;
 // try
//  {
   fromCanonPath = jEdit.StringGet(from,jEdit.getCanonPath());// _from.getCanonicalPath();
//  }
//  catch(IOException io)
//  {
//   fromCanonPath = from;
//  }
   System.out.println("FileVFS _rename line 378 fromCanonPath= "+fromCanonPath+" toCanonPath= "+toCanonPath);
  // Case-insensitive fs workaround
 // long time=System.currentTimeMillis(); long timecurr=time;
   // while (timecurr-time<3000) timecurr=System.currentTimeMillis();
    if(!fromCanonPath.equalsIgnoreCase(toCanonPath))
   jEdit.BoolGet(to,jEdit.getDelete());//_to.delete();
  boolean retVal =jEdit.RenameGet(from,jEdit.getRenameTo(),to);// _from.renameTo(_to);
  VFSManager.sendVFSUpdate(this,fromCanonPath,true);
  VFSManager.sendVFSUpdate(this,toCanonPath,true);
    System.out.println("FileVFS _rename line 385 from(savePath)= "+from+" to= "+to+" retVal= "+retVal);
  return retVal;
 } //}}}

 //{{{ _mkdir() method
 public boolean _mkdir(Object session, String directory, Component comp)
 {
  String parent = getParentOfPath(directory);
  if(!jEdit.BoolGet(parent,jEdit.getExists()))//new File(parent).exists())
  {
   if(!_mkdir(session,parent,comp))
    return false;
  }

  File file = new File(directory);

  boolean retVal = jEdit.BoolGet(directory,jEdit.getMkDir());//file.mkdir();
  String canonPath;
//  try
//  {
   canonPath = jEdit.StringGet(directory,jEdit.getCanonPath());//file.getCanonicalPath();
//  }
//  catch(IOException io)
//  {
//   canonPath = directory;
//  }
  VFSManager.sendVFSUpdate(this,canonPath,true);
  return retVal;
 } //}}}

 //{{{ _backup() method
 public void _backup(Object session, String path, Component comp)
  throws IOException
 {
  // Fetch properties
  int backups = jEdit.getIntegerProperty("backups",1);

  if(backups == 0)
   return;

  String backupPrefix = jEdit.getProperty("backup.prefix");
  String backupSuffix = jEdit.getProperty("backup.suffix");

  String backupDirectory = jEdit.getProperty("backup.directory");

  int backupTimeDistance = jEdit.getIntegerProperty("backup.minTime",0);
  File file = new File(path);
    String parent = file.getParent();
    if (File.separatorChar!=jEdit.separatorChar) parent=parent.replace(File.separatorChar,jEdit.separatorChar);
    if (!jEdit.BoolGet(path,jEdit.getExists()))//file.exists())
   return;

  // Check for backup.directory, and create that
  // directory if it doesn't exist
  if(backupDirectory == null || backupDirectory.length() == 0)
   backupDirectory = parent;
  else
  {
   backupDirectory = MiscUtilities.constructPath(
   jEdit.getJEditHome(),backupDirectory);// System.getProperty("user.home"),backupDirectory);

   // Perhaps here we would want to guard with
   // a property for parallel backups or not.
   backupDirectory = MiscUtilities.concatPath(
    backupDirectory,parent);

   // File dir = new File(backupDirectory);

   if (!jEdit.BoolGet(backupDirectory,jEdit.getExists()))//dir.exists())
    jEdit.BoolGet(backupDirectory,jEdit.getMkDirs());//dir.mkdirs();
  }

  MiscUtilities.saveBackup(path,backups,backupPrefix,
   backupSuffix,backupDirectory,backupTimeDistance);
 } //}}}

 //{{{ _createInputStream() method
 public InputStream _createInputStream(Object session, String path,
  boolean ignoreErrors, Component comp) throws IOException
 {
  try
  {
   return new FileInputStream(path);
  }
  catch(IOException io)
  {
   if(ignoreErrors)
    return null;
   else
    throw io;
  }
 } //}}}

 //{{{ _createOutputStream() method
 public OutputStream _createOutputStream(Object session, String path,
  Component comp) throws IOException
 {
  return new FileOutputStream(path);
 } //}}}

 //{{{ _saveComplete() method
 public void _saveComplete(Object session, Buffer buffer, String path,
  Component comp)
 {
  int permissions = buffer.getIntegerProperty(PERMISSIONS_PROPERTY,0);
  setPermissions(path,permissions);
 } //}}}

 //{{{ Permission preservation code

 /** Code borrowed from j text editor (http://www.armedbear.org) */
 /** I made some changes to make it support suid, sgid and sticky files */

 //{{{ getPermissions() method
 /**
  * Returns numeric permissions of a file. On non-Unix systems, always
  * returns zero.
  * @since jEdit 3.2pre9
  */
 public static int getPermissions(String path)
 {
  int permissions = 0;

  if(jEdit.getBooleanProperty("chmodDisabled"))
   return permissions;
    permissions=(int)jEdit.LongGet(path,jEdit.getPermissions());

  /*  if(OperatingSystem.isUnix())
  {  String[] cmdarray = { "ls", "-ld", path };
   try {  Process process = Runtime.getRuntime().exec(cmdarray);
    BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
    String output = reader.readLine();
    if(output != null)
    { String s = output.substring(1, 10);
     permissions = MiscUtilities.parsePermissions(s);
    }
   }
   // Feb 4 2000 5:30 PM
   // Catch Throwable here rather than Exception.
   // Kaffe's implementation of Runtime.exec throws java.lang.InternalError.
   catch (Throwable t)
   { t.printStackTrace();
   }
  }  */

  return permissions;
 } //}}}

 //{{{ setPermissions() method
 /**
  * Sets numeric permissions of a file. On non-Unix platforms,
  * does nothing.
  * @since jEdit 3.2pre9
  */
 public static void setPermissions(String path, int permissions)
 {
  if(jEdit.getBooleanProperty("chmodDisabled"))
   return;

  if(permissions != 0)
  {
   if(OperatingSystem.isUnix())
   {
     jEdit.SetPermissionsGet(path,18,permissions);//18=SetPermissions command
  /*        //this block moved to servlet
     String[] cmdarray = { "chmod", Integer.toString(permissions, 8), path };
    try
    {
     Process process = Runtime.getRuntime().exec(cmdarray);
     process.getInputStream().close();
     process.getOutputStream().close();
     process.getErrorStream().close();
     // Jun 9 2004 12:40 PM
     // waitFor() hangs on some Java
     // implementations.
     /* int exitCode = process.waitFor();
     if(exitCode != 0)
      Log.log(Log.NOTICE,FileVFS.class,"chmod exited with code " + exitCode); */
   // }

    // Feb 4 2000 5:30 PM
    // Catch Throwable here rather than Exception.
    // Kaffe's implementation of Runtime.exec throws java.lang.InternalError.
   /* catch (Throwable t)
    {
    } */
   }
  }
 } //}}}

 //}}}

 //{{{ Private members
 private static FileSystemView fsView = FileSystemView.getFileSystemView();
 //}}}
}
