package ru.sibinco.scag.beans.rules.applet;


import ru.sibinco.scag.backend.SCAGAppContext;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpServlet;
import javax.servlet.ServletException;
import java.io.*;
import java.util.LinkedList;
import java.util.zip.GZIPOutputStream;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 13.07.2005
 * Time: 19:12:51
 * To change this template use File | Settings | File Templates.
 */
public class myServlet extends HttpServlet
{
  protected static final int Delete=1;
  protected static final int CanRead=2;
  protected static final int IsDirectory=3;
  protected static final int MkDir=4;
  protected static final int MkDirs=5;
  protected static final int Exists=6;
  protected static final int ListFiles=7;
  protected static final int IsHidden=8;
  protected static final int CanonPath=9;
  protected static final int ParseXml=10;
  protected static final int LastModifed=11;
  protected static final int CanWrite=12;
  protected static final int Length=13;
  protected static final int NotHiddenFiles=14;
  protected static final int RenameTo=15;
  protected static final int SaveBackup=16;
  protected static final int Write=17;
  protected static final int SetPermissions=18;
  protected static final int GetPermissions=19;
  protected static final int FileEncoding = 20;
  protected static final int LineSeparator = 21;
  protected static final int SeparatorChar = 22;
  protected static final int OsName = 23;
  protected static final int Transport = 24;

  // public static String userdir=null;
  // private static String settingsDirectory;
  public final void doGet(HttpServletRequest req,
                          HttpServletResponse res)
          throws ServletException,IOException
  { String file=req.getParameter("file");
    String[] list=null;
    LinkedList li;
    int command=Integer.parseInt(req.getParameter("command"));
   // System.out.println("myServlet Get file= "+file+" command= "+command);
    res.setContentType("text/html; charset=windows-1251");
    if(file!=null) {
       switch (command)
    {
       case ParseXml: li=ParseXml(file); String status=(String)li.get(0);res.setHeader("status",status);
        if (status.equals("ok")) { PrintWriter out = res.getWriter();
          for (int i = 1; i < li.size(); i++)
            out.println(li.get(i)); //  System.out.println("myServlet li["+i+"]= "+li.get(i)+" command= "+command);
          out.flush(); out.close();
        }   break;
      case Transport : list=getTransport(file,req);break;
      case SaveBackup: list=SaveBackup(new File(file), req); break;
      default:
        if (req.getParameter("renameto")!=null) list=RenameTo(new File(file),new File(req.getParameter("renameto")));
        else if (req.getParameter("intparam")!=null) list=FilesCommand(file,command,Integer.parseInt(req.getParameter("intparam")));
        else list=FilesCommand(new File(file),command);
    }

        if (list!=null) {
        PrintWriter out = res.getWriter();
        System.out.println("myServlet file= "+file+" command= "+command);
        for (int i = 0; i < list.length; i++)
          out.println(list[i]); //if (command==14)  System.out.println("myServlet list["+i+"]= "+list[i]+" command= "+command);
        out.flush(); out.close();
      }
    }
    //  doRequest(req, res);
  }

  public final void doPost(HttpServletRequest req,
                           HttpServletResponse res)
          throws ServletException,IOException
  {
    doRequest(req, res);
  }

  public final void doPut(HttpServletRequest req,
                          HttpServletResponse res)
          throws ServletException,IOException
  { String file=req.getParameter("file");
    int command=Integer.parseInt(req.getParameter("command"));
    System.out.println("myServlet PUT file= "+file+" command= "+command);
    PrintWriter out = res.getWriter();
    res.setContentType("text/html; charset=windows-1251");
    if (command==Write) {
      System.out.println("doPut file="+file);
      OutputStream _out=null;
      BufferedReader r=req.getReader();
      String s; BufferedWriter buf=null;
      try {
        _out = new FileOutputStream(file);
        if(file.endsWith(".gz")) _out = new GZIPOutputStream(_out);
        buf = new BufferedWriter(new OutputStreamWriter(_out));

        while((s=r.readLine())!=null) { //li.add(s);
          //System.out.println(s); 
          buf.write(s); buf.newLine();
        }
      } catch (FileNotFoundException e) { e.printStackTrace();
      } catch (IOException e) {  e.printStackTrace();
      }
      finally{
        try {
          if (buf!=null) buf.close();
          if (_out!=null) _out.close();
        } catch (IOException e) { e.printStackTrace(); }
      }

    }  // if (command==Write)
    out.print("true");out.flush();out.close();
    //doRequest(req, res);
  }


  private LinkedList ParseXml(final String fileName)
  {
    LinkedList li=new  LinkedList();
    InputStream _in=null; BufferedReader in = null; String inpuLine;
    try {  _in = new FileInputStream(fileName); in = new BufferedReader(new InputStreamReader(_in));
      li.addFirst("ok");
      while ((inpuLine = in.readLine()) != null) li.add(inpuLine);
    } catch (FileNotFoundException e) { e.printStackTrace();
      li.addFirst(e.getMessage());
    } catch (IOException e) {  e.printStackTrace();
    }
    finally{
      try {
        if (in!=null) in.close(); if (_in!=null) _in.close();
      } catch (IOException e) { e.printStackTrace(); }
    }
    return li;
  }

    private LinkedList ParseXml(final InputStream inputStream) {
        LinkedList li = new LinkedList();
        BufferedReader bufferedReader = null;
        String inpuLine;
        try {
            bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
            li.addFirst("ok");
            while ((inpuLine = bufferedReader.readLine()) != null) li.add(inpuLine);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            li.addFirst(e.getMessage());
        } catch (IOException e) {
            e.printStackTrace();
        }
        finally {
            try {
                if (bufferedReader != null) bufferedReader.close();
                if (inputStream != null) inputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return li;
    }


    private String[] FilesCommand(final String path,final int command,int parameter)
  {
    String result="false";
    String[] list=new String[1]; 
    list[0]=result;
    switch (command)
    {
      case SetPermissions :if (MiscUtilities.setPermissions(path,parameter)) result="true";
        list[0]=result; return list;
      default: if (FileCommand(new File(path),command)) result="true";list[0]=result; return list;
    }
  }
  private String[] FilesCommand(final File autosaveFile,final int command)
  {
    //LinkedList list=new  LinkedList();
    String result="false";
    String[] list=new String[1];
    list[0]=result;
    switch (command)
    {
      case ListFiles      : list=autosaveFile.list();
        // if (list !=null) System.out.println("myServlet FilesCommand list= "+list+"length= "+list.length+"  list[0]= "+list[0]);
        // if (list!=null && list.length>1) System.out.println("myServlet FilesCommand list[1]= "+list[1]);
        return list;
      case NotHiddenFiles ://FileSystemView fsView = FileSystemView.getFileSystemView();
        System.out.println("myServlet getNotHiddenFiles autosaveFile.getPath= "+autosaveFile.getPath());
        File[] files=autosaveFile.listFiles();//fsView.getFiles(autosaveFile,false);
        LinkedList li=new LinkedList();
        for (int i = 0; i < files.length; i++) {
          File file = files[i];
          if (!file.isHidden()) {
            li.add(file.getPath());System.out.println("myServlet getNotHiddenFiles file.getPath= "+file.getPath());
          }
        }
        list=new String[li.size()];
        for (int i = 0; i < li.size(); i++) {
          list[i] = (String) li.get(i);
          System.out.println("myServlet getNotHiddenFiles list[i]= "+list[i]);
        }
        return list;
      case CanonPath      :
        try {  list[0]=autosaveFile.getCanonicalPath(); }
        catch (IOException e) { e.printStackTrace(); list[0]=autosaveFile.getPath(); }
        return list;
      case FileEncoding   : list[0]=System.getProperty("file.encoding"); return list;
      case LineSeparator  : list[0]=System.getProperty("line.separator"); return list;
      case OsName         : list[0]=System.getProperty("os.name"); return list;
      case SeparatorChar  : list[0]=String.valueOf(File.separatorChar); return list;
      case LastModifed    : long last=autosaveFile.lastModified();
        result=String.valueOf(last);list[0]=result;return list;
      case Length         : long len=autosaveFile.length();
        result=String.valueOf(len);list[0]=result;return list;
      case GetPermissions : long permissions=MiscUtilities.getPermissions(autosaveFile.getPath());
        result=String.valueOf(permissions);list[0]=result;return list;
      default: if (FileCommand(autosaveFile,command)) result="true";list[0]=result; return list;
    }

  }
  private String[] RenameTo(final File autosaveFile, final File renameto)
  {
    String result="false";
    System.out.println("RenameTo from "+autosaveFile.getPath()+" to "+renameto.getPath());
    String[] list=new String[1];
    list[0]=result;
    if (autosaveFile.renameTo(renameto)) result="true";
    list[0]=result; System.out.println("result= "+result);return list;
  }

  private String[] getTransport(String id,HttpServletRequest req) {
    String result="false";
    String[] list=new String[1];
    list[0]=result;
    SCAGAppContext appContext = (SCAGAppContext) req.getAttribute("appContext");
    list[0]=appContext.getRuleManager().getRuleTransportDir(id); //transport
    return list;
  }
  private String[] SaveBackup(final File source, HttpServletRequest req)
  {
    String result="false";
    String[] list=new String[1];
    list[0]=result;
    int backups=Integer.parseInt(req.getParameter("bs"));
    String backupPrefix=req.getParameter("bPrx");
    String backupSuffix=req.getParameter("bSix");
    String backupDirectory=req.getParameter("bDir");
    int  backupTimeDistance=Integer.parseInt(req.getParameter("bTmDst"));
    MiscUtilities.saveBackup(source,backups,backupPrefix,backupSuffix,backupDirectory,backupTimeDistance);
    result="true";  list[0]=result; return list;
  }

  private boolean FileCommand(final File autosaveFile,final int command)
  {
    switch (command)
    {
      case Delete      : return autosaveFile.delete() ;
      case CanRead     : return autosaveFile.canRead();
      case IsDirectory : return autosaveFile.isDirectory();
      case MkDir       : return autosaveFile.mkdir();
      case MkDirs      : return autosaveFile.mkdirs();
      case Exists      : return autosaveFile.exists();
      case IsHidden    : return autosaveFile.isHidden();
      case CanWrite    : return autosaveFile.canWrite();
      default: return false;
    }
  }
      private void doRequest(HttpServletRequest req,
                         HttpServletResponse res)

  { try{ res.setContentType("text/html; charset=windows-1251");
    PrintWriter out = res.getWriter();
    out.println("<html><body>");
    out.println("<h1>Hello, world !</h1>");
    out.println("</body></html>");
    out.flush();
    out.close();
  } catch(Exception e) { e.printStackTrace();
  }
  }
 
}

