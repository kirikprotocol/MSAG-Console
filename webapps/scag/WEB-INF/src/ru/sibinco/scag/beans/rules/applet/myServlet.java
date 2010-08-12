package ru.sibinco.scag.beans.rules.applet;


import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.beans.rules.RuleState;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpSession;
import javax.servlet.ServletException;
import java.io.*;
import java.util.LinkedList;
import java.util.zip.GZIPOutputStream;

import org.apache.log4j.Logger;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 13.07.2005
 * Time: 19:12:51
 * To change this template use File | Settings | File Templates.
 */
public class myServlet extends HttpServlet
{
  protected final Logger logger = Logger.getLogger(this.getClass());

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
  protected static final int NewRule = 25;
  protected static final int UpdateRule = 26;
  protected static final int ExistRule = 27;
  protected static final int LoadRule = 28;
  protected static final int LoadNewRule = 29;
  protected static final int AddRule = 30;
  protected static final int RootElement = 31;
  protected static final int Title = 32;
  protected static final int UnLockServiceRule = 34;
  protected static final int RuleHeaderLineNumber = 35;
  protected static final int getRuleStateAndLock = 36;
  protected HttpSession session = null;

  // public static String userdir=null;
  // private static String settingsDirectory;
  public final void doGet(HttpServletRequest req,
                          HttpServletResponse res)
          throws IOException
  {
    String file=req.getParameter("file");
    String transport=null;
    String[] list=null;
    LinkedList li;
    int command=Integer.parseInt(req.getParameter("command"));
    if (file.indexOf(Rule.complexRuleIdSeparator)!=-1) {
     String[] id_transport = Rule.getIdAndTransport(file);
     file = id_transport[0];
     transport = id_transport[1];
    }

   // System.out.println("myServlet Get file= "+file+" command= "+command);
    res.setContentType("text/html; charset=utf-8");
    if(file!=null) {
       switch (command)
    {
       case ParseXml:   li=ParseXml(file); SendResult(li,res); break;
       case RootElement:li=RootElement(req);   SendResult(li,res); break;
       case LoadRule:   li=LoadRule(req,file,transport); SendResult(li,res); break;
       case LoadNewRule:li=LoadNewRule(req,file,transport); SendResult(li,res); break;
       case ExistRule:  ExistRule(req,file,transport,res); break;
       case Transport : list=getTransport(transport);break;
       case SaveBackup: list=SaveBackup(new File(file), req); break;
       case UnLockServiceRule: unlockRule(req,file,transport); break;
       case Title: getTitle(req,file,transport,res); break;
       case RuleHeaderLineNumber: getRuleHeaderLineNumber(res); break;
       case getRuleStateAndLock: getRuleStateAndLock(req,file,transport,res); break;
       default:
        if (req.getParameter("renameto")!=null) list=RenameTo(new File(file),new File(req.getParameter("renameto")));
        else if (req.getParameter("intparam")!=null) list=FilesCommand(file,command,Integer.parseInt(req.getParameter("intparam")));
        else list=FilesCommand(new File(file),command);
    }

        if (list!=null) {
        PrintWriter out = res.getWriter();
       // System.out.println("myServlet file= "+file+" command= "+command);
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
          throws IOException
  {
    String file=req.getParameter("file");
    String transport=null;
    int command=Integer.parseInt(req.getParameter("command"));
    if (file.indexOf(Rule.complexRuleIdSeparator)!=-1) {
     String[] id_transport = Rule.getIdAndTransport(file);
     file = id_transport[0];
     transport = id_transport[1];
    }
    res.setContentType("text/html; charset=utf-8");
    if (command==Write) Write(req,file,res);
    if (command==UpdateRule) updateRule(req,file,transport,res);
    if (command==AddRule) AddRule(req,file,transport,res);
  }
 private void Write(HttpServletRequest req,final String file,HttpServletResponse res) throws IOException
 {
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
             if (r!=null) r.close();
             if (_out!=null) _out.close();
           } catch (IOException e) { e.printStackTrace(); }
         }
   PrintWriter out = res.getWriter();
   out.print("true");out.flush();out.close();
 }

  private void unlockRule(HttpServletRequest req,final String file, final String transport) {
     SCAGAppContext appContext = (SCAGAppContext) req.getAttribute(Constants.APP_CONTEXT);
     appContext.getRuleManager().unlockRule(file, transport.toUpperCase());
  }

  private void getTitle(HttpServletRequest req, final String file, final String transport, HttpServletResponse res) {
     SCAGAppContext appContext = (SCAGAppContext) req.getAttribute(Constants.APP_CONTEXT);
     try {
      String serviceName=appContext.getServiceProviderManager().getServiceById(new Long(file)).getName();
      if (serviceName!=null) {        
        String total = transport + ", " + serviceName + "(id=" + file + ")";
        res.getWriter().write(total);
        res.getWriter().flush();
        res.getWriter().close();
      }
     } catch(Throwable e) {
     }
  }

  private void getRuleHeaderLineNumber(HttpServletResponse res) {
     int lineCount = Rule.mainHeaderLength;
     try {
       res.getWriter().write(""+lineCount);
       res.getWriter().flush();
       res.getWriter().close();
     } catch (Throwable e) {
       e.printStackTrace();
     }
  }

  private void getRuleStateAndLock(HttpServletRequest req, final String file, final String transport, HttpServletResponse res) {
    SCAGAppContext appContext = (SCAGAppContext) req.getAttribute(Constants.APP_CONTEXT);
    try {
     RuleState ruleState = appContext.getRuleManager().getRuleStateAndLock(file,transport);                
     ObjectOutputStream out = new ObjectOutputStream(res.getOutputStream());
     out.writeObject(ruleState);
     out.flush();
     out.close();
    } catch(Throwable e) {
      e.printStackTrace();
    }
  }

  private void updateRule(HttpServletRequest req,final String file, final String transport, HttpServletResponse res) throws IOException
  {
    SCAGAppContext appContext = (SCAGAppContext) req.getAttribute(Constants.APP_CONTEXT);
    //transport
    LinkedList li;
    PrintWriter out = res.getWriter();
    BufferedReader r=req.getReader();
    try {
      li=appContext.getRuleManager().updateRule(r,file,transport, RuleManager.NON_TERM_MODE, req.getUserPrincipal().getName());
      res = checkError(li, res);
      out.print("true");out.flush();out.close();
    }  catch (SibincoException e) {
      res.setHeader("status",e.getMessage());
      if (e instanceof StatusDisconnectedException)   {
        res.setHeader("servicestatus","0");
        out.println("true");
      } else {
        out.println("false");
        out.println(e.getMessage());
      }
        out.flush();out.close();
    }    finally {
      if (r!=null)
        try {
          r.close();
        } catch (IOException e)  { e.printStackTrace(); }
    }
 }
  private void AddRule(HttpServletRequest req,final String file, final String transport, HttpServletResponse res) throws IOException
  {
    SCAGAppContext appContext = (SCAGAppContext) req.getAttribute(Constants.APP_CONTEXT);
    PrintWriter out = res.getWriter();    
    LinkedList li;
    BufferedReader r=req.getReader();
    try {
       li = appContext.getRuleManager().AddRule(r,file,transport,RuleManager.NON_TERM_MODE, req.getUserPrincipal().getName());
       res = checkError(li, res);
       out.print("true");out.flush();out.close();
    } catch (SibincoException e) {
      res.setHeader("status",e.getMessage());
      if (e instanceof StatusDisconnectedException) {
        res.setHeader("servicestatus","0");
        out.println("true");
      } else {
        out.println("false");
        out.print(e.getMessage());
      }
      out.flush();out.close();
    } finally {
      try {
        if (r!=null) r.close();
      } catch (IOException e) { e.printStackTrace(); }
    }
  }

  private HttpServletResponse checkError(LinkedList li, HttpServletResponse res)
  {
    int errorType=0;
    int lineIndex=0;
    int start=0;
    int end=0;String error;
    if (li != null && li.size() > 0)
    {
         /*errorType=Integer.parseInt((String)li.get(0));*/  res.setIntHeader("errorType",errorType);
         lineIndex=Integer.parseInt((String)li.get(1));      res.setIntHeader("lineIndex",lineIndex);
         /*start=Integer.parseInt((String)li.get(2));*/      res.setIntHeader("start",start);
         /*end=Integer.parseInt((String)li.get(3)); */       res.setIntHeader("end",end);
         error=(String)li.get(0);                            res.setHeader("error",error);
         res.setHeader("status","error");
     }
     else
     res.setHeader("status","ok");
     return res;
  }
  private LinkedList LoadRule(HttpServletRequest req, final String file, final String transport)
    {
      logger.debug("LoadRule id= "+file+" transport="+transport);
      SCAGAppContext appContext = (SCAGAppContext) req.getAttribute(Constants.APP_CONTEXT);
      Rule rule = appContext.getRuleManager().getRule(file ,transport);
      return rule.getBody();
    }

  private LinkedList LoadNewRule(HttpServletRequest req, final String file, final String transport)
  {
    logger.debug("LoadNewRule id= "+file + " transport = "+transport);
    Rule newRule=Rule.createNewRule(Long.parseLong(file),transport);
    LinkedList li;
    li=newRule.getBody();
    logger.debug("body size = " + li.size());
    if(li.size()>0) {
      li.addFirst("ok");
    } else
       li.add("error: newRule is null!");
    return li;
  }

   private void ExistRule(HttpServletRequest req, final String file, final String transport,HttpServletResponse res) throws IOException
  {
    logger.debug("ExistRule id= "+file + " transport="+transport);
    SCAGAppContext appContext = (SCAGAppContext) req.getAttribute(Constants.APP_CONTEXT);
    PrintWriter out = res.getWriter();
    if (appContext.getRuleManager().checkRuleFileExists(file,transport))
        out.print("true");
    else  out.print("false");
    out.flush();out.close();
  }

  private LinkedList ParseXml(final String fileName )
  {
    LinkedList li=new  LinkedList();
    InputStream _in=null; BufferedReader in = null; String inpuLine;
    try {  _in = new FileInputStream(fileName); in = new BufferedReader(new InputStreamReader(_in));
      li.addFirst("ok");
      while ((inpuLine = in.readLine()) != null) {
        //System.out.println("$$$"+inpuLine);
        li.add(inpuLine);
      }

    } catch (FileNotFoundException e) {
      //e.printStackTrace();
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
  private LinkedList RootElement(HttpServletRequest req)
  {
    LinkedList li=new LinkedList();
    li.add(Rule.ROOT_ELEMENT);
    if(li.size()>0) li.addFirst("ok");
    else li.add("error: Root Element is not specified !");
    return li;
  }

  private void SendResult(LinkedList li,HttpServletResponse res) throws IOException
  {
  String  status=(String)li.get(0);res.setHeader("status",status);
  if (status.equals("ok")) { PrintWriter out = res.getWriter();
          for (int i = 1; i < li.size(); i++)
            out.println(li.get(i)); //  System.out.println("myServlet li["+i+"]= "+li.get(i)+" command= "+command);
          out.flush(); out.close();
        }
  }
    private LinkedList ParseRule(final InputStream inputStream)
    {
        LinkedList li = new LinkedList();
        BufferedReader bufferedReader = null;String inpuLine;
        try {
            bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
            li.addFirst("ok");
            while ((inpuLine = bufferedReader.readLine()) != null) li.add(inpuLine);
        } catch (FileNotFoundException e) { // e.printStackTrace();
            li.addFirst(e.getMessage());
        } catch (IOException e) { e.printStackTrace();
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
       // System.out.println("myServlet getNotHiddenFiles autosaveFile.getPath= "+autosaveFile.getPath());
        File[] files=autosaveFile.listFiles();//fsView.getFiles(autosaveFile,false);
        LinkedList li=new LinkedList();
        for (int i = 0; i < files.length; i++) {
          File file = files[i];
          if (!file.isHidden()) {
            li.add(file.getPath());//System.out.println("myServlet getNotHiddenFiles file.getPath= "+file.getPath());
          }
        }
        list=new String[li.size()];
        for (int i = 0; i < li.size(); i++) {
          list[i] = (String) li.get(i);
        //  System.out.println("myServlet getNotHiddenFiles list[i]= "+list[i]);
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
    //System.out.println("RenameTo from "+autosaveFile.getPath()+" to "+renameto.getPath());
    String[] list=new String[1];
    list[0]=result;
    if (autosaveFile.renameTo(renameto)) result="true";
    list[0]=result; System.out.println("result= "+result);return list;
  }

  private String[] getTransport(final String transport) {
    return new String[]{transport};
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

  { try{ res.setContentType("text/html; charset=utf-8");
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

