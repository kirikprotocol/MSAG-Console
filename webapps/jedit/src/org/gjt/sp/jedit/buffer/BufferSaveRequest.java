/*
 * BufferSaveRequest.java - I/O request
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 2000, 2005 Slava Pestov
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

package org.gjt.sp.jedit.buffer;

//{{{ Imports
import javax.swing.text.Segment;
import java.io.*;
import java.nio.charset.Charset;
import java.util.zip.*;
import java.util.Vector;
import java.util.LinkedList;
import java.awt.*;
import java.net.URL;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;

import org.gjt.sp.jedit.io.*;
import org.gjt.sp.jedit.*;
import org.gjt.sp.util.*;
//}}}

/**
 * A buffer save request.
 * @author Slava Pestov
 * @version $Id$
 */
public class BufferSaveRequest extends BufferIORequest
{
	//{{{ BufferSaveRequest constructor
	/**
	 * Creates a new buffer I/O request.
	 * @param view The view
	 * @param buffer The buffer
	 * @param session The VFS session
	 * @param vfs The VFS
	 * @param path The path
	 */
	public BufferSaveRequest(View view, Buffer buffer,
		Object session, VFS vfs, String path)
	{
		super(view,buffer,session,vfs,path);
	} //}}}

	public void _createOutputStream(Object session, String path,
		Component comp,Buffer buffer) throws IOException
	{

    URL url=null;
    HttpURLConnection c=null;
    BufferedReader in=null;  int command=jEdit.getWrite();
    String content="?username="+jEdit.username+"&password="+jEdit.password+"&command="+command+"&file="+path;
    String inputLine="";    OutputStream _out=null;
    LinkedList list=new  LinkedList();
    try {
      url=new URL(jEdit.servletUrl,content);
      c=(HttpURLConnection) url.openConnection();
      c.setDoOutput(true);
      c.setRequestMethod("PUT");
     // c.setRequestProperty("Content-Length","100");
      c.connect();
      _out=c.getOutputStream();
      } catch (MalformedURLException e) {
      e.printStackTrace();
      }
      if(_out == null)
					return;
  try {
      writeMarkers(buffer,_out);
      } catch (IOException e) {
      e.printStackTrace();
      }
	} //}}}

	//{{{ run() method
	public void run()
	{
		OutputStream out = null;
		try
		{
			String[] args = { vfs.getFileName(path) };
			setStatus(jEdit.getProperty("vfs.status.save",args));

			// the entire save operation can be aborted...
			setAbortable(true);

			path = vfs._canonPath(session,path,view);			if(!MiscUtilities.isURL(path))
				path = MiscUtilities.resolveSymlinks(path);

			// Only backup once per session
			if(buffer.getProperty(Buffer.BACKED_UP) == null
				|| jEdit.getBooleanProperty("backupEverySave"))
			{
        vfs._backup(session,path,view);
				buffer.setBooleanProperty(Buffer.BACKED_UP,true);
			}

			/* if the VFS supports renaming files, we first
			 * save to #<filename>#save#, then rename that
			 * to <filename>, so that if the save fails,
			 * data will not be lost.
			 *
			 * as of 4.1pre7 we now call vfs.getTwoStageSaveName()
			 * instead of constructing the path directly
			 * since some VFS's might not allow # in filenames.
			 */
			String savePath;

			boolean twoStageSave = (vfs.getCapabilities() & VFS.RENAME_CAP) != 0
				&& jEdit.getBooleanProperty("twoStageSave");
			if(twoStageSave)
			{
				savePath = vfs.getTwoStageSaveName(path);
				if (savePath == null)
				{
					twoStageSave = false;
					savePath = path;
				}
			}
			else
				savePath = path;
	//		out = vfs._createOutputStream(session,savePath,view);
        InputStream _in=null; BufferedReader in = null;
    URL url; HttpURLConnection c=null; int command=jEdit.getWrite();
    String content="?username="+jEdit.username+"&password="+jEdit.password+"&command="+command+"&file="+savePath;
    try {
          url=new URL(jEdit.servletUrl,content);
          c=(HttpURLConnection) url.openConnection();
        } catch (MalformedURLException e) {
             e.printStackTrace();
        }
         c.setDoOutput(true); c.setRequestMethod("PUT");
          //c.setRequestProperty("Content-Length","100");
          c.connect(); out=c.getOutputStream();

			try
			{
				// this must be after the stream is created or
				// we deadlock with SSHTools.
				buffer.readLock();
				if(out != null)
				{
					// Can't use buffer.getName() here because
					// it is not changed until the save is
					// complete
					if(savePath.endsWith(".gz"))
						buffer.setBooleanProperty(Buffer.GZIPPED,true);

					if(buffer.getBooleanProperty(Buffer.GZIPPED))
						out = new GZIPOutputStream(out);
					write(buffer,out);
          if(out != null) out.close();
          _in = c.getInputStream(); // _in = new FileInputStream(path);
      in = new BufferedReader(new InputStreamReader(_in));//new FileReader(fileName));
          //String status=c.getHeaderField("status");
      //if (status.equals("ok"))
       String status=in.readLine();
		//	else throw new FileNotFoundException(status);
     System.out.println("BufferSaveRequest run line 178 status= "+status);
           if(_in != null) _in.close();
           if(in != null) in.close();
          if (c!=null) c.disconnect();
        System.out.println("BufferSaveRequest run line 182 path= "+path+" savePath= "+savePath);
					if(twoStageSave)
					{
						if(!vfs._rename(session,savePath,path,view))
							throw new IOException("Rename failed: " + savePath);
					}

					// We only save markers to VFS's that support deletion.
					// Otherwise, we will accumilate stale marks files.
					if((vfs.getCapabilities() & VFS.DELETE_CAP) != 0)
					{
						if(jEdit.getBooleanProperty("persistentMarkers")
							&& buffer.getMarkers().size() != 0)
						{
							setStatus(jEdit.getProperty("vfs.status.save-markers",args));
							setProgressValue(0);
							//out = vfs._createOutputStream(session,markersPath,view);
              _createOutputStream(session,markersPath,view,buffer);
						//	if(out != null)
						//		writeMarkers(buffer,out);
						}
						else
							vfs._delete(session,markersPath,view);
					}
				}
				else
					buffer.setBooleanProperty(ERROR_OCCURRED,true);

				if(!twoStageSave)
					VFSManager.sendVFSUpdate(vfs,path,true);
			}
			finally
			{
          if(_in != null) _in.close();
          if(in != null) in.close();
				buffer.readUnlock();
			}
		}
		catch(IOException io)
		{
		//	Log.log(Log.ERROR,this,io);
			String[] pp = { io.toString() };
			VFSManager.error(view,path,"ioerror.write-error",pp);

			buffer.setBooleanProperty(ERROR_OCCURRED,true);
		}
		catch(WorkThread.Abort a)
		{

				if(out != null)
			{
				try
				{
					out.close();
				}
				catch(IOException io)
				{ io.printStackTrace();
				}
			}

			buffer.setBooleanProperty(ERROR_OCCURRED,true);
		}
		finally
		{
			try
			{
				vfs._saveComplete(session,buffer,path,view);
				vfs._endVFSSession(session,view);
			}
			catch(IOException io)
			{
			//	Log.log(Log.ERROR,this,io);
				String[] pp = { io.toString() };
				VFSManager.error(view,path,"ioerror.write-error",pp);

				buffer.setBooleanProperty(ERROR_OCCURRED,true);
			}
			catch(WorkThread.Abort a)
			{
				buffer.setBooleanProperty(ERROR_OCCURRED,true);
			}
		}
	} //}}}

	//{{{ writeMarkers() method
	private void writeMarkers(Buffer buffer, OutputStream out)
		throws IOException
	{
		Writer o = new BufferedWriter(new OutputStreamWriter(out));
		try
		{
			Vector markers = buffer.getMarkers();
			for(int i = 0; i < markers.size(); i++)
			{
				Marker marker = (Marker)markers.elementAt(i);
				o.write('!');
				o.write(marker.getShortcut());
				o.write(';');

				String pos = String.valueOf(marker.getPosition());
				o.write(pos);
				o.write(';');
				o.write(pos);
				o.write('\n');
			}
		}
		finally
		{
			o.close();
		}
	} //}}}
}
