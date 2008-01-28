package ru.sibinco.scag.backend.rules;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;
import ru.sibinco.WHOISDIntegrator.RuleManagerWrapper;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.installation.HSDaemon;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.rules.RuleState;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;
import java.text.DateFormat;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 19.08.2005
 * Time: 14:42:35
 * To change this template use File | Settings | File Templates.
 */


public class RuleManager
{
    public static final String SMPP_TRANSPORT_NAME = "SMPP";
    public static final String HTTP_TRANSPORT_NAME = "HTTP";
    public static final String MMS_TRANSPORT_NAME = "MMS";
    public final static int NON_TERM_MODE = 0;
    public final static int TERM_MODE = 1;
    private final RuleManagerWrapper wrapper;
    private final Map schemas = Collections.synchronizedMap(new HashMap());
    private final Map ruleStates = Collections.synchronizedMap(new HashMap());
    private final File rulesFolder;
    private final File xsdFolder;
    private final File xslFolder;
    private final Scag scag;
    private final HSDaemon hsDaemon;
    private final static Object lockObject = new Object();
    private final Logger logger = Logger.getLogger(this.getClass());
    private final static boolean DEBUG = false;

    private boolean savePermissionSMPP = true;
    private boolean savePermissionHTTP = true;
    private boolean savePermissionMMS  = true;

    public static final String SPACE4 = "    ";

    public void setSavePermissionSMPP(boolean savePermissionSMPP) {
        this.savePermissionSMPP = savePermissionSMPP;
    }

    public void setSavePermissionHTTP(boolean savePermissionHTTP) {
        this.savePermissionHTTP = savePermissionHTTP;
    }

    public void setSavePermissionMMS(boolean savePermissionMMS) {
        this.savePermissionMMS = savePermissionMMS;
    }

  public RuleManager(final File rulesFolder,final File xsdFolder, final File xslFolder, final Scag scag, final HSDaemon hsDaemon) {
    this.rulesFolder = rulesFolder;
    this.xsdFolder = xsdFolder;
    this.xslFolder = xslFolder;
    this.scag =scag;
    this.hsDaemon = hsDaemon;
    wrapper = new RuleManagerWrapper(this);
  }

  public void init()
          throws SibincoException
  {
    load();
  }

  public RuleManagerWrapper getWrapper() {
    wrapper.clear();
    return wrapper;
  }

  public File getXslFolder()
  {
    return xslFolder;
  }


  public File composeRuleFile(String transport, String serviceId) {
      String filename="rule_"+ serviceId.toString()+".xml";
      final File folder = new File(rulesFolder, transport);
      File newFile= new File(folder,filename);
      return newFile;
  }

  public File composeRuleBackFile(String transport, String serviceId, String backSuffix ) {
      if( backSuffix == null ) backSuffix = "";
      String filename="rule_"+ serviceId.toString() + ".xml" + backSuffix;
      final File folder = new File(rulesFolder, transport);
      File newFile= new File(folder,filename);
      return newFile;
  }

  public boolean checkRuleFileExists(String ruleId, String transport) {
    return composeRuleFile(transport, ruleId).exists();
  }

  public Rule getRule(String ruleId, String transport)
  {
    Rule rule = null;
    File ruleFile = composeRuleFile(transport, ruleId);
    if (ruleFile.exists())
      try {
        rule = LoadRule(transport,ruleFile);
      } catch (Exception e) {
        return null;
      }
    return rule;
  }

  //SYNCHRONIZATION
  public RuleState getRuleState(String ruleId, String transport) {
    synchronized (lockObject) {
      String complexRuleId = Rule.composeComplexId(ruleId,transport);
      RuleState ruleState = null;
      ruleState = (RuleState)ruleStates.get(complexRuleId);
      if (ruleState!=null) return ruleState;
      ruleState = new RuleState();
      ruleState.setExists(checkRuleFileExists(ruleId, transport));
      ruleState.setLocked(false);
      ruleStates.put(complexRuleId,ruleState);
      return ruleState;
    }
  }

  //this method is called when client want to lock rule!
  public RuleState getRuleStateAndLock(String ruleId, String transport) {
    synchronized (lockObject) {
      String complexRuleId = Rule.composeComplexId(ruleId,transport);
      RuleState ruleState = null;
      ruleState = (RuleState)ruleStates.get(complexRuleId);
      if (ruleState!=null) {
        if (ruleState.getLocked()) {
          return ruleState;
        } else {
          RuleState copy = ruleState.copy();
          //lock rule!!!
          ruleState.setLocked(true);
          //return copy - whith unlocked state - client don't need to lock rule!
          return copy;
        }
      } else {
        ruleState = new RuleState();
        ruleState.setExists(checkRuleFileExists(ruleId, transport));
        ruleState.setLocked(false);
        RuleState copy = ruleState.copy();
        //lock rule!!!
        ruleState.setLocked(true);
        ruleStates.put(complexRuleId,ruleState);
        //return copy - when unlocked state - client don't need to lock rule!
        return copy;
      }
    }
  }


  private void setRuleState(String ruleId, String transport, boolean exists, boolean locked) {
    synchronized (lockObject) {
      String complexRuleId = Rule.composeComplexId(ruleId,transport);
      RuleState ruleState = (RuleState)ruleStates.get(complexRuleId);
      ruleState.setExists(exists);
      ruleState.setLocked(locked);
    }
  }

  public void unlockRule(String ruleId, String transport) {
    System.out.println("!!!!!unlock rule id = " + ruleId + " transport = " + transport);
    setRuleState(ruleId, transport, checkRuleFileExists(ruleId, transport), false);
  }

  public void unlockRule(String complexRuleId) {
      System.out.println("!!!!!unlock rule complexString='" + complexRuleId + "'" );
    String[] id_transport = Rule.getIdAndTransport(complexRuleId);
    unlockRule(id_transport[0], id_transport[1]);
  }

  public void load() throws SibincoException
  {
    try {
      File folder = null;
      for(int i=0;i<Transport.transportTitles.length;i++) {
        folder = new File(rulesFolder,Transport.transportTitles[i]);
        if(!folder.exists())
          folder.mkdirs();
      }
      loadSchemas();
      Object[] headerInfo = loadRuleHeader();
      Rule.mainHeaderLength = ((Integer)headerInfo[0]).intValue();
      Rule.header = (String)headerInfo[1];
    } catch (SibincoException e) {
      e.printStackTrace();
      throw new SibincoException(e.getMessage());
    }
  }

  private Object[] loadRuleHeader() throws SibincoException {
    File headerTemplatePath = new File(rulesFolder,"rule_header.template");
    StringBuffer header = new StringBuffer();
    FileInputStream fs = null;
    BufferedReader br = null;
    int lineCounter = 0;
    try {
     fs = new FileInputStream(headerTemplatePath);
     br = new BufferedReader(new InputStreamReader(fs,"UTF-8"));
     String line;
     while ((line = br.readLine())!=null) {
      header.append(line).append('\n');
      lineCounter++;
     }
    } catch (Exception e) {
      e.printStackTrace();
      throw new SibincoException(e.getMessage());
    } finally {
      try {
      if (fs!=null)  fs.close();
      if (br!=null)  br.close();
      } catch(IOException e){e.printStackTrace();}
    }
    return new Object[]{new Integer(lineCounter),header.toString()};
  }

  private void loadSchemas()
  {
     String[] schemasfiles = xsdFolder.list(new FilenameFilter(){
       public boolean accept(File dir, String name){
         return (name.endsWith(".xsd") || name.endsWith(".dtd"));
       }
     });
     for(int i = 0; i<schemasfiles.length;i++) {
      loadSchema(schemasfiles[i]);
    }
  }

  private Schema loadSchema(String schemaPath) {
    InputStream in = null; BufferedReader br = null;
    Schema schema = null;
    try {
    LinkedList schemaContent = new LinkedList();
    File schemaFile =new File(xsdFolder,schemaPath);
    in = new FileInputStream(schemaFile);
    br = new BufferedReader(new InputStreamReader(in,"UTF-8"));
    String line;
    while ((line=br.readLine())!=null)
      schemaContent.add(line);
      schema = new Schema(schemaFile.lastModified(),schemaContent);
      schemas.put(schemaPath, schema);
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (IOException io) {
      io.printStackTrace();
    } finally {
      try {
       if (in!=null) in.close();
       if (br!=null) br.close();
      }
      catch (IOException e) {
        e.printStackTrace();
      }
    }
    return schema;
  }

  private Rule LoadRule(String _transportDir, File file) throws SibincoException {
    Rule temp;
    String fileName=file.getName();
    if (!fileName.endsWith(".xml")) return null;

    logger.debug("enter " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
    LinkedList body=LoadXml(file.getAbsolutePath());
    //long length=file.length();
    //System.out.println("RuleManager.LoadFromFolder file.length= "+length);
    try {
    final Document ruleDoc = Utils.parse(file.getAbsolutePath());
    Element el=ruleDoc.getDocumentElement();
    if (!el.getTagName().equals(Rule.ROOT_ELEMENT))
      throw new SibincoException("Root element "+el.getTagName()+" not math with "+Rule.ROOT_ELEMENT);
    String transport=el.getAttribute("transport");
    if (transport.equals(""))
      throw new SibincoException("Root element "+el.getTagName()+" not contain attribute 'transport'");
    String ruleId=fileName.substring(5,fileName.length()-4);
    String notes="";
    Long id=Long.decode(ruleId);

    temp = new Rule(id,notes,transport,body);
    logger.debug("exit " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");

} catch (FactoryConfigurationError error) {
  logger.error("Couldn't configure xml parser factory", error);
  throw new SibincoException("Couldn't configure xml parser factory", error);
} catch (ParserConfigurationException e) {
  logger.error("Couldn't configure xml parser", e);
  throw new SibincoException("Couldn't configure xml parser", e);
} catch (SAXException e) {
  String fileId=fileName.substring(5,fileName.length()-4);
  Long id=Long.decode(fileId);
  String notes="Load Error "+e.getMessage();
  temp = new Rule(id, notes, _transportDir,body/*,length*/);

  logger.error("Couldn't parse", e);
  //System.out.println("RuleManager LoadFromFile fileName= "+fileName+" SAXException e= "+e.getMessage());
      // throw new SibincoException("Couldn't parse", e);
} catch (IOException e) {
  logger.error("Couldn't perfmon IO operation", e);
  throw new SibincoException("Couldn't perfmon IO operation", e);
} catch (NullPointerException e) {
  logger.error("Couldn't parse", e);
  throw new SibincoException("Couldn't parse", e);
}
   return temp;
  }

  private LinkedList LoadXml(final String fileName)
   {
     LinkedList li=new  LinkedList();
     InputStream _in=null;
     BufferedReader in = null;
     String inputLine;
     long length=0;
     try {
       _in = new FileInputStream(fileName);
       in = new BufferedReader(new InputStreamReader(_in,"UTF-8"));
       li.addFirst("ok");
       while ((inputLine = in.readLine()) != null) {
         li.add(inputLine);length+=inputLine.length();
       }
     } catch (FileNotFoundException e) {
       e.printStackTrace();
       li.addFirst(e.getMessage());
     } catch (IOException e) {
       e.printStackTrace();
     }
     finally {
       try {
         if (in!=null) in.close();
         if (_in!=null) _in.close();
       } catch (IOException e) {
         e.printStackTrace(); }
     }
     logger.debug("RuleManager.LoadXml length= "+length);
     return li;
   }

   public LinkedList getSchema(String requestedScheme)
   {
     File requestedSchemeFile = new File(xsdFolder,requestedScheme);
     Schema schema = (Schema)schemas.get(requestedScheme);
     if (requestedSchemeFile.lastModified() > schema.lastmodified) {
      schema = loadSchema(requestedScheme);
     }
     return schema.schemaContent;
   }

 //-----------------------------------------commands methods------------------------------------------------------//
 public synchronized LinkedList AddRule(BufferedReader r, final String ruleId, final String transport, int mode, String user) throws SibincoException, IOException
 {
  logger.debug("AddNewRule ruleId= "+ruleId + " ,transport = "+transport);
  LinkedList errorInfo=new LinkedList();

  //instead of scag.addRule(ruleId, transport);
  if (DEBUG) {
    String buffer = buffertostring(r);
    r = new BufferedReader(new StringReader(buffer));
    errorInfo = errorsImitator(r);
    r = new BufferedReader(new StringReader(buffer));
  }

  saveRule(r,ruleId, transport);

  if (!DEBUG) {
  try {
    errorInfo =(LinkedList) scag.addRule(ruleId, transport);
  } catch (SibincoException e) {
    if (!(e instanceof StatusDisconnectedException)) {
      if (mode!=TERM_MODE) saveMessage("Failed to add rule: ", user, ruleId, transport);
      removeRuleFile(ruleId, transport);
    } else {
      if (mode!=TERM_MODE) {
        finishOperation(ruleId,transport,HSDaemon.UPDATEORADD);
        saveMessage("Added rule: ", user, ruleId, transport);
      }
    }
    logger.error(e.getMessage());// e.printStackTrace();
    throw e;
   }
  }

  if (errorInfo == null || errorInfo.size() == 0) {
    if (mode!=TERM_MODE) {
      finishOperation(ruleId,transport,HSDaemon.UPDATEORADD);
      saveMessage("Added rule: ", user, ruleId, transport);
    }
  }  else  {
    if (mode!=TERM_MODE) saveMessage("Failed to add rule: ", user, ruleId, transport);
    removeRuleFile(ruleId, transport);
  }
  return errorInfo;
}

  public synchronized LinkedList updateRule(BufferedReader r, final String ruleId, final String transport, int mode, String user) throws SibincoException,  IOException
 {
   //String ruleId=fileName.substring(5,fileName.length()-4);
   Rule rule = getRule(ruleId,transport);
   logger.debug("updateRule ruleId= "+ruleId+" ,transport = "+transport);
   LinkedList errorInfo = new LinkedList();

   //instead of scag.updateRule(ruleId, transport);
   if (DEBUG) {
    String buffer = buffertostring(r);
    r = new BufferedReader(new StringReader(buffer));
    errorInfo = errorsImitator(r);
    r = new BufferedReader(new StringReader(buffer));
   }

   LinkedList curbody = rule.getBody();
   // rule_1.xml -> rule_1.xml.new(rule_1.xml.new contains current rule body)
   File currentRuleFile = saveCurrentRule(curbody,ruleId, transport);
   saveRule(r,ruleId, transport);

   if (!DEBUG)  {
   try    {
     errorInfo=(LinkedList) scag.updateRule(ruleId, transport);
   }    catch (SibincoException e)    {
     if (e instanceof StatusDisconnectedException) {
       if (mode != TERM_MODE) {
           logger.error("RuleManager:StatusDisconnectedException:if:1");
         Functions.SavedFileToBackup(currentRuleFile,".new");
         finishOperation(ruleId,transport,HSDaemon.UPDATEORADD);
         saveMessage("Updated rule: ", user, ruleId, transport);
       }
     } else {
       if (mode != TERM_MODE) {
        saveRule(curbody,ruleId,rule.getTransport());
        currentRuleFile.delete();
        saveMessage("Failed to update rule: ", user, ruleId, transport);
       }
     }
     logger.error(e.getMessage());// e.printStackTrace();
     throw e;
    }
   }
    if (errorInfo == null || errorInfo.size()==0) {
        if (mode!=TERM_MODE) {
            logger.error("RuleManager:StatusDisconnectedException:if2:");
          Functions.SavedFileToBackup(currentRuleFile, ".new");
          finishOperation(ruleId,transport,HSDaemon.UPDATEORADD);
          saveMessage("Updated rule: ", user, ruleId, transport);
        }
    } else {
       if (mode != TERM_MODE) {
         saveMessage("Failed to update rule: ", user, ruleId, transport);
         saveRule(curbody,ruleId,rule.getTransport());
         currentRuleFile.delete();
       }
    }
   return errorInfo;
 }

  public synchronized void removeRule(final String ruleId, final String transport, int mode, String user) throws SibincoException
  {
        //in term mode file deleted in removeRuleCommit(...) method in RuleManagerWrapper
        try {
          scag.removeRule(ruleId,transport);
          if (mode != TERM_MODE) {
            removeRuleFile(ruleId, transport);
            finishOperation(ruleId,transport,HSDaemon.REMOVE);
            saveMessage("Removed rule: ", user, ruleId, transport);
          }
        } catch (SibincoException se) {
          if (se instanceof StatusDisconnectedException) {
            if (mode != TERM_MODE) {
             removeRuleFile(ruleId, transport);
             finishOperation(ruleId,transport,HSDaemon.REMOVE);
             saveMessage("Removed rule: ", user, ruleId, transport);
            }
          } else {
            saveMessage("Failed to remove rule: ", user, ruleId, transport);
            throw se;
          }
        }
  }
    boolean checkPermission( String transport){
//        logger.debug( "RuleManager:checkPermission()" );
        if( transport.equals(SMPP_TRANSPORT_NAME)){
            return savePermissionSMPP;
        }else if( transport.equals(HTTP_TRANSPORT_NAME) ){
            return savePermissionHTTP;
        }else if( transport.equals(MMS_TRANSPORT_NAME) ){
            return savePermissionMMS;
        }else{
            return false;
        }
    }

    private File saveCurrentRule(LinkedList li,String ruleId, String transport) {
        File ruleFile;
        File newFile = null;
        if( checkPermission(transport) ){
            logger.debug("RuleManger:saveCurrentRule( , , )");
            ruleFile = composeRuleFile(transport,ruleId);
            newFile = Functions.createNewFilenameForSave(ruleFile);
            try {
              PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), "UTF-8"));
              for (Iterator i = li.listIterator(1); i.hasNext();) {
                  out.println(i.next());
              }
              out.flush();
              out.close();
            } catch (Exception e) {e.printStackTrace();}
        } else {
            logger.error( "Attempt to save unlocked rule. RuleManger:saveCurrentRule( , , ):" +
                          "permission=false for " + transport );
        }
        return newFile;
    }

  public void saveRule(LinkedList li,String ruleId, String transport) throws SibincoException
  {
    if( checkPermission(transport) ){
        logger.debug("RESAVING current rule body to disc!!!!");
        try {
            File newFile= composeRuleFile(transport,ruleId);
            final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), "UTF-8"));
            for (Iterator i = li.listIterator(1); i.hasNext();)
                out.println(i.next());
            out.flush();
            out.close();
        } catch (FileNotFoundException e) {
            throw new SibincoException("Couldn't save new rule : Couldn't write to destination config filename: " + e.getMessage());
        } catch (IOException e) {
            logger.error("Couldn't save new rule settings", e);
            throw new SibincoException("Couldn't save new rule template", e);
        }
    }else{
        logger.error( "Attempt to save unlocked rule.RuleManger:saveRule( LL, S, S):" +
                      "permission=false for " + transport );
    }
  }

    public static int CHAR_WRITER = 0;
    public static int STRING_WRITER = 1;

  private void saveRule(BufferedReader reader,String ruleId, String transport) throws SibincoException
  {
      PrintWriter out = null;
      File ruleFile = null;
      File tempFile = null;
//      File tempFile1 = null;
//      File backFile = null;
      try
      {
        if( checkPermission(transport) ){
            logger.debug("Saving rule to disc!!!! file : " + ruleFile);
            ruleFile = composeRuleFile(transport,ruleId);

            tempFile = composeRuleBackFile( transport, ruleId, ".tmp" );
            out = new PrintWriter( new OutputStreamWriter( new FileOutputStream(tempFile), "UTF-8" ) ); //alter out = new PrintWriter( new FileWriter(newFile) );
            ruleWriter( reader, out, STRING_WRITER );

//            backFile = new File( tempFile.getAbsolutePath().substring(0, tempFile.getAbsolutePath().lastIndexOf("/")),
//                                 tempFile.getName()+Functions.suffixDateFormat.format(new Date()) );
//            Functions.checkCreateBackupDir( ruleFile );
//            tempFile1 = composeRuleBackFile( transport, ruleId, ".tmp1" );

            out.flush();
            out.close();

            logger.debug(" Move rule to backup: " + ruleFile);
//            Functions.MoveFileToBackupDir( ruleFile, "");
            logger.debug(" Move temp rule to rule: " + tempFile);
            tempFile.renameTo( ruleFile );

        } else {
            logger.error( "Attempt to save unlocked rule. RuleManger:saveRule( BR, S, S):" +
                        "permission=false for " + transport );
        }
      }
      catch (FileNotFoundException e) {
        throw new SibincoException("Couldn't save new rule : Couldn't write to destination config filename: " + e.getMessage());
      }
      catch (IOException e){
        out.flush();
        out.close();
//        tempFile.delete();    //          Functions.
        logger.error("Couldn't save new rule settings", e);
        throw new SibincoException("Couldn't save new rule template", e);
      }
    }

    void ruleWriter( BufferedReader r, PrintWriter out, int type ) throws IOException {
        switch( type ){
            case 0:
               System.out.println("CHAR WRITER");
               logger.info( "CHAR WRITER" );
               int ch;
               Reader reader = r;
                System.getProperties();
               while( (ch = reader.read()) != -1 ){
                   if(ch=='\n')
                   out.print( (char)ch );
               }
                break;
            case 1:
                System.out.println("STRING WRITER");
                String s;
                StringBuffer sb = new StringBuffer();
                sb.delete(0,sb.length());
                while( (s=r.readLine() ) != null ) {
                    int tabIndex = 0;
                    while( ( tabIndex=s.indexOf( "\t") )!=-1 ){
                        s = s.substring(0,tabIndex) + SPACE4 + s.substring(tabIndex+1);
                    }
                    out.println(s);
                }
                break;
            default:
                break;
        }
    }


    public void saveBackup( File source, File target ){
        RandomAccessFile input = null;
        RandomAccessFile output = null;
        try {
            input = new RandomAccessFile( source, "r");
            output = new RandomAccessFile( target, "rw");
        } catch (FileNotFoundException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }
        try{
            byte[] buf = new byte[32768];
            long length = input.length();
            output.setLength( length );
            int bytesRead;
            while( (bytesRead =input.read(buf, 0, buf.length)) != -1 ){
                output.write( buf, 0, bytesRead );
            }
        }catch( IOException e ){
            try{ input.close();  } catch( Exception e1 ){};
            try{ output.close(); } catch( Exception e1 ){};
            return;
        }
        try{ input.close(); }catch( IOException e1 )  { logger.error( "IOException while closing file '" + source.getName() + "'" ); }
        try{ output.close(); }catch( IOException e1 ) { logger.error( "IOException while closing file '" + target.getName() + "'" ); }
    }

    public void restoreFile( File backupFile, File targetFile ){
        logger.error( "Restored rule in file '" + targetFile.getName() + "' from file '" + backupFile + "'" );
        saveBackup( backupFile, targetFile );
    }

    public void saveMessage(final String stringMessage, final String user, final String ruleId, final String transport) {
      StatMessage message = new StatMessage(user, "Rule", stringMessage + "rule_"+ ruleId+".xml for transport " + transport);
      StatusManager.getInstance().addStatMessages(message);
    }

    public void removeRulesForService(final String user,final String ruleId) throws SCAGJspException
    {
      //TODO - check if some one else have deleted service!
      String[] transports = Transport.transportTitles;
      for (byte i =0 ;i<transports.length;i++) {
          RuleState ruleState = getRuleStateAndLock(ruleId,transports[i]);
          if (ruleState.getLocked()) throw new SCAGJspException(ru.sibinco.scag.Constants.errors.services.COULDNT_DELETE_SERVICE_RULE_IS_EDITING,ruleId);
          try {
            if (ruleState.getExists()) removeRule(ruleId, transports[i], NON_TERM_MODE, user);
            ruleStates.remove(Rule.composeComplexId(ruleId,transports[i]));
          } catch(SibincoException se) {
              se.printStackTrace();/*PRINT ERROR ON THE SCREEN;*/
              unlockRule(ruleId, transports[i]);
              logger.error("Couldn't remove rule");
              throw new SCAGJspException(ru.sibinco.scag.Constants.errors.rules.COULD_NOT_REMOVE_RULE, se);
          }
      }
   }

    public void removeRuleFile(String ruleId, String transport) throws SibincoException
    {
        File fileForDeleting= composeRuleFile(transport,ruleId);
        logger.debug("Trying to delete : " + fileForDeleting);
        if (fileForDeleting.exists())
          if (fileForDeleting.delete()!=true) throw new SibincoException("Couldn't delete rule " + fileForDeleting.getAbsolutePath());
    }

    public void finishOperation(String ruleId, String transport, byte operationType) throws SibincoException {
      File ruleFile= composeRuleFile(transport,ruleId);
      hsDaemon.doOperation(ruleFile,operationType);
    }

  private String buffertostring(BufferedReader r) throws IOException
  {
      String temp = "";
      String line;
      while ((line = r.readLine())!=null)
      {
          temp= temp + line +"\n";
      }
      return temp;
  }

  private LinkedList errorsImitator(BufferedReader r) throws IOException
  {
      LinkedList error=new LinkedList();

      //search for forbidden constants, e.g. @dummy, @foo...
      String[] forbiddenconsts = {"@dummy","@foo"};
      int tt=-1;
      String line;
      while ((line = r.readLine())!=null)
      {
          tt++;
          for (int ii = 0;ii<forbiddenconsts.length;++ii)
          if (line.indexOf(forbiddenconsts[ii])!=-1)
          {
             error.add("RuleEngineException exception: Wrong constant: "+forbiddenconsts[ii]);
             error.add(""+tt);
             return error;
          }
      }
      return null;
  }
  public static class Schema {
    public long lastmodified;
    public LinkedList schemaContent;
    public Schema(long lastmodified, LinkedList schemaContent) {
      this.lastmodified = lastmodified;
      this.schemaContent = schemaContent;
    }
  }
}

