/*
 * BufferAutosaveRequest.java - I/O request
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
import java.util.Iterator;
import java.util.LinkedList;
import java.awt.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.MalformedURLException;

import org.gjt.sp.jedit.io.*;
import org.gjt.sp.jedit.*;
import org.gjt.sp.util.*;
//}}}

/**
 * A buffer autosave request.
 * @author Slava Pestov
 * @version $Id$
 */
public class BufferAutosaveRequest extends BufferIORequest
{
	//{{{ BufferAutosaveRequest constructor
	/**
	 * Creates a new buffer I/O request.
	 * @param view The view
	 * @param buffer The buffer
	 * @param session The VFS session
	 * @param vfs The VFS
	 * @param path The path
	 */
	public BufferAutosaveRequest(View view, Buffer buffer,
		Object session, VFS vfs, String path)
	{
		super(view,buffer,session,vfs,path);
	} //}}}

	//{{{ run() method
	public void run()
	{
		OutputStream out = null;

		try
		{
			String[] args = { vfs.getFileName(path) };
			setStatus(jEdit.getProperty("vfs.status.autosave",args));

			// the entire save operation can be aborted...
			setAbortable(true);

			try
			{
				//buffer.readLock();

				if(!buffer.isDirty())
				{
					// buffer has been saved while we
					// were waiting.
					return;
				}

				//out = vfs._createOutputStream(session,path,view);
				 _createOutputStream(session,path,view,buffer);
       // System.out.println("BufferAutosaveRequest after createOutputStream out= "+out);
      //  if(out == null)
			//		return;

			//	write(buffer,out);
			}
			catch(Exception e)
			{
			}
			finally
			{
				//buffer.readUnlock();
			}
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
				{
				}
			}
		}
	} //}}}
  	public void _createOutputStream(Object session, String path,
		Component comp,Buffer buffer) throws IOException
	{

    URL url=null;
    HttpURLConnection c=null;
    BufferedReader in=null;  int command=jEdit.getWrite();
    String content="?username="+jEdit.username+"&password="+jEdit.password+"&command="+command+"&file="+path;
    String inputLine="";
    LinkedList list=new  LinkedList();
    try {
      url=new URL(jEdit.servletUrl,content);
      c=(HttpURLConnection) url.openConnection();
      c.setDoOutput(true);
      c.setRequestMethod("PUT");
      //c.setRequestProperty("Content-Length","10");
      c.connect();
      OutputStream _out=c.getOutputStream();
      if(_out == null)
					return;
      write(buffer,_out);
      _out.close();
      in = new BufferedReader( new InputStreamReader(c.getInputStream()));
      while ((inputLine = in.readLine()) != null)
       list.add(inputLine);
    } catch (MalformedURLException e) {
      e.printStackTrace();
      /*  }  catch (ProtocolException e) {
      e.printStackTrace();
      */ } catch (IOException e) {
      e.printStackTrace();
    }
    finally{
      try {
      if (in!=null) in.close();
      if (c!=null) c.disconnect();
        } catch (IOException e) {
          e.printStackTrace();
        }
    }
	} //}}}
}
