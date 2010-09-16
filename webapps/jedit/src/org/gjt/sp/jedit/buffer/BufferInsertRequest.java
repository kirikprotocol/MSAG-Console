/*
 * BufferInsertRequest.java - I/O request
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
import java.net.URL;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;

import org.gjt.sp.jedit.io.*;
import org.gjt.sp.jedit.*;
import org.gjt.sp.util.*;
//}}}

/**
 * A buffer insert request.
 * @author Slava Pestov
 * @version $Id$
 */
public class BufferInsertRequest extends BufferIORequest
{
 //{{{ BufferInsertRequest constructor
 /**
  * Creates a new buffer I/O request.
  * @param view The view
  * @param buffer The buffer
  * @param session The VFS session
  * @param vfs The VFS
  * @param path The path
  */
 public BufferInsertRequest(View view, Buffer buffer,
  Object session, VFS vfs, String path)
 {
  super(view,buffer,session,vfs,path);
 } //}}}
  public SegmentBuffer _createInputStream(Object session, String path,
  long length) throws IOException ,FileNotFoundException
 {

    URL url=null;
    HttpURLConnection c=null;InputStream _in=null;
    int command=jEdit.getParseXml();
    String content="?username="+jEdit.username+"&password="+jEdit.password+"&command="+command+"&file="+path;
    SegmentBuffer seg=null;
    try {
      url=new URL(jEdit.servletUrl,content);
      c=(HttpURLConnection) url.openConnection();
      System.out.println("BufferInsertRequest open connection: url="+url);
      c.connect(); String status=c.getHeaderField("status");
      System.out.println("BufferInsertRequest _createInputStream status= "+status);
      if (!status.equals("ok")) throw new FileNotFoundException(status);
      _in=c.getInputStream();
      if(_in == null)
     return null;

        long startTime = System.currentTimeMillis();

        seg=read(autodetect(_in),length,false);

        int currentTime=(int)(System.currentTimeMillis()-startTime);
        System.out.println("BufferInsertRequest read time:"+currentTime+" ms");

    } catch (MalformedURLException e) {
      e.printStackTrace();
      /*  }  catch (ProtocolException e) {
      e.printStackTrace();
      */ }
    finally{
      try {
      if (_in!=null) _in.close();
      if (c!=null) c.disconnect();
        } catch (IOException e) {
          e.printStackTrace();
        }
    }
    return seg;
 } //}}}
 //{{{ run() method
 public void run()
 {
  InputStream in = null;

  try
  {
   try
   {
    String[] args = { vfs.getFileName(path) };
    setStatus(jEdit.getProperty("vfs.status.load",args));
    setAbortable(true);

    path = vfs._canonPath(session,path,view);

    VFSFile entry = vfs._getFile(
     session,path,view);
    long length;
    if(entry != null)
     length = entry.getLength();
    else
     length = 0L;

   // in = vfs._createInputStream(session,path,false,view);
  //  if(in == null)
  //   return;
       final SegmentBuffer seg=_createInputStream(session,path,length);
  //  final SegmentBuffer seg = read(
  //   autodetect(in),length,true);

    /* we don't do this in Buffer.insert() so that
       we can insert multiple files at once */
    VFSManager.runInAWTThread(new Runnable()
    {
     public void run()
     {
      view.getTextArea().setSelectedText(
       seg.toString());
     }
    });
   }
   catch(IOException io)
   {
   // Log.log(Log.ERROR,this,io);
    String[] pp = { io.toString() };
    VFSManager.error(view,path,"ioerror.read-error",pp);

    buffer.setBooleanProperty(ERROR_OCCURRED,true);
   }
  }
  catch(WorkThread.Abort a)
  {
   if(in != null)
   {
    try
    {
     in.close();
    }
    catch(IOException io)
    {
    }
   }

   buffer.setBooleanProperty(ERROR_OCCURRED,true);
  }
  finally
  {
   try
   {
    vfs._endVFSSession(session,view);
   }
   catch(IOException io)
   {
   // Log.log(Log.ERROR,this,io);
    String[] pp = { io.toString() };
    VFSManager.error(view,path,"ioerror.read-error",pp);

    buffer.setBooleanProperty(ERROR_OCCURRED,true);
   }
   catch(WorkThread.Abort a)
   {
    buffer.setBooleanProperty(ERROR_OCCURRED,true);
   }
  }
 } //}}}
}
