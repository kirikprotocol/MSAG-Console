/*
 * BufferLoadRequest.java - I/O request
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
import sidekick.SideKickPlugin;
import errorlist.DefaultErrorSource;
//}}}

/**
 * A buffer load request.
 * @author Slava Pestov
 * @version $Id$
 */
public class BufferLoadRequest extends BufferIORequest
{
  //{{{ BufferLoadRequest constructor
  /**
   * Creates a new buffer I/O request.
   * @param view The view
   * @param buffer The buffer
   * @param session The VFS session
   * @param vfs The VFS
   * @param path The path
   */
  public BufferLoadRequest(View view, Buffer buffer,
                           Object session, VFS vfs, String path)
  {
    super(view,buffer,session,vfs,path);
  } //}}}

  public void _createInputStream(Object session, String path,
                                 Buffer buffer, long length,boolean markers) throws IOException ,FileNotFoundException
  {

    URL url=null;
    HttpURLConnection c=null;InputStream _in=null;
    int command=jEdit.getParseXml();
    if (jEdit.getBooleanProperty("bufferWorkWithId")) command=jEdit.getLoadRule();
    if (buffer!=null && buffer.getBooleanProperty("newRule")) command=jEdit.getLoadNewRule();
    String content="?username="+jEdit.username+"&password="+jEdit.password+"&command="+command+"&file="+path;
    System.out.println("BufferLoadRequest _createInputStream content= "+content);
    try {
      url=new URL(jEdit.servletUrl,content);
      c=(HttpURLConnection) url.openConnection();
      System.out.println("BufferLoadRequest open connection: url="+url);
      c.connect(); String status=c.getHeaderField("status");
      /* if (jEdit.getBooleanProperty("bufferWorkWithId")) {

        } */
      //this is hack!!! trying to get c.getHeaderField("status") again if its's null
      for (int i=10000;i<30000;i++){
        if (((i%10000)==0) && status != null) break;
          else if (i%10000==0) status = c.getHeaderField("status");
      }
      System.out.println("BufferLoadRequest _createInputStream status= "+status);
      if (!status.equals("ok")) throw new FileNotFoundException(status);
      _in=c.getInputStream();
      if(_in == null)
        return;

      long startTime = System.currentTimeMillis();
      if (!markers) {
          read(autodetect(_in),length,false);
      } else {
          readMarkers(buffer,_in);
      }
      int currentTime=(int)(System.currentTimeMillis()-startTime);
      System.out.println("BufferLoadRequest read time:"+currentTime+" ms");

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
  } //}}}

  //{{{ run() method
  public void run()
  {
    InputStream in = null;

    try
    {
      try
      {
        setAbortable(true);
        long length = 0;
        if (!jEdit.getBooleanProperty("bufferWorkWithId")) {

          String[] args = { vfs.getFileName(path) };
          if(!buffer.isTemporary())
          {
            setStatus(jEdit.getProperty("vfs.status.load",args));
            setProgressValue(0);
          }

          path = vfs._canonPath(session,path,view);

          VFSFile entry = vfs._getFile(session,path,view);
          if(entry != null)
            length = entry.getLength();
          else
            length = 0L;

        }
        /*  in = vfs._createInputStream(session,path,
       false,view);
      if(in == null)
       return;
        */
        _createInputStream(session,path,buffer,length,false);
        //read(autodetect(in),length,false);
        buffer.setNewFile(false);
      }
      catch(CharConversionException ch)
      {
        //  Log.log(Log.ERROR,this,ch);
        Object[] pp = { buffer.getProperty(Buffer.ENCODING),
                ch.toString() };
        VFSManager.error(view,path,"ioerror.encoding-error",pp);
        ch.printStackTrace();
        buffer.setBooleanProperty(ERROR_OCCURRED,true);
      }
      catch(UnsupportedEncodingException uu)
      {
        //  Log.log(Log.ERROR,this,uu);
        Object[] pp = { buffer.getProperty(Buffer.ENCODING),
                uu.toString() };
        VFSManager.error(view,path,"ioerror.encoding-error",pp);
        uu.printStackTrace();
        buffer.setBooleanProperty(ERROR_OCCURRED,true);
      }
      catch(IOException io)
      {
        // Log.log(Log.ERROR,this,io);
        Object[] pp = { io.toString() };
        VFSManager.error(view,path,"ioerror.read-error",pp);
        io.printStackTrace();
        buffer.setBooleanProperty(ERROR_OCCURRED,true);
      }
      catch(OutOfMemoryError oom)
      {
        // Log.log(Log.ERROR,this,oom);
        VFSManager.error(view,path,"out-of-memory-error",null);
        oom.printStackTrace();
        buffer.setBooleanProperty(ERROR_OCCURRED,true);
      }

      if(jEdit.getBooleanProperty("persistentMarkers") && !jEdit.getBooleanProperty("bufferWorkWithId"))
      {
        try
        {
          String[] args = { vfs.getFileName(path) };
          if(!buffer.isTemporary())
            setStatus(jEdit.getProperty("vfs.status.load-markers",args));
          setAbortable(true);

          // in = vfs._createInputStream(session,markersPath,true,view);
          _createInputStream(session,markersPath,buffer,0,true);
          // if(in != null)
          // readMarkers(buffer,in);
        }
        catch(IOException io)
        {
          // ignore
        }
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

  //{{{ readMarkers() method
  private void readMarkers(Buffer buffer, InputStream _in)
          throws IOException
  {
    // For `reload' command
    buffer.removeAllMarkers();

    BufferedReader in = new BufferedReader(new InputStreamReader(_in));

    try
    {
      String line;
      while((line = in.readLine()) != null)
      {
        // compatibility kludge for jEdit 3.1 and earlier
        if(!line.startsWith("!"))
          continue;

        // malformed marks file?
        if(line.length() == 0)
          continue;

        char shortcut = line.charAt(1);
        int start = line.indexOf(';');
        int end = line.indexOf(';',start + 1);
        int position = Integer.parseInt(line.substring(start + 1,end));
        buffer.addMarker(shortcut,position);
      }
    }
    finally
    {
      in.close();
    }
  } //}}}
}
