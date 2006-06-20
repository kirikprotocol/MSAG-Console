package ru.sibinco.scag.backend.rules;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;
import ru.sibinco.lib.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 19.08.2005
 * Time: 14:42:35
 * To change this template use File | Settings | File Templates.
 */


public class RuleManager
{
  public final static int NON_TERM_MODE = 0;
  public final static int TERM_MODE = 1;
  private final Map schemas = Collections.synchronizedMap(new HashMap());
  private final File rulesFolder;
  private final File xsdFolder;
  private final File xslFolder;
  private final Scag scag;
  private final Logger logger = Logger.getLogger(this.getClass());
  private final static boolean DEBUG = false;

  public RuleManager(final File rulesFolder,final File xsdFolder, final File xslFolder, final Scag scag) {
    this.rulesFolder = rulesFolder;
    this.xsdFolder = xsdFolder;
    this.xslFolder = xslFolder;
    this.scag =scag;
  }

  public void init()
          throws SibincoException
  {
    load();
  }

  public File getXslFolder()
  {
    return xslFolder;
  }


  public File composeRuleFile(String transport, Long serviceId) {
      String filename="rule_"+ serviceId.toString()+".xml";
      final File folder = new File(rulesFolder, transport);
      File newFile= new File(folder,filename);
      return newFile;
  }

  public Rule getRule(Long ruleId, String transport)
  {
    Rule rule = null;
    File ruleFile = composeRuleFile(transport, ruleId);
    if (ruleFile.exists())
      try {
      rule = LoadRule(transport,ruleFile);
      } catch (Exception e) {return null;}
    return rule;
  }

 public LinkedList getRuleBody(Long ruleId, String transport)
 {
   Rule rule = getRule(ruleId, transport);
   return rule.getBody();
 }

  public synchronized void load() throws SibincoException
  {
    try {
      File folder = null;
      for(int i=0;i<Transport.transportTitles.length;i++) {
        folder = new File(rulesFolder,Transport.transportTitles[i]);
        if(!folder.exists())
          folder.mkdirs();
      }
      loadSchemas();
      Rule.header = loadRuleHeader();
    } catch (SibincoException e) {
      e.printStackTrace();
      throw new SibincoException(e.getMessage());
    }
  }

  private String loadRuleHeader() throws SibincoException {
    File headerTemplatePath = new File(rulesFolder,"rule_header.template");
    StringBuffer header = new StringBuffer();
    FileInputStream fs = null;
    BufferedReader br = null;
    try {
     fs = new FileInputStream(headerTemplatePath);
     br = new BufferedReader(new InputStreamReader(fs));
     String line;
     while ((line = br.readLine())!=null) {
      header.append(line).append('\n');
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
    return header.toString();
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
    br = new BufferedReader(new InputStreamReader(in));
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
     InputStream _in=null; BufferedReader in = null; String inputLine;long length=0;
     try {  _in = new FileInputStream(fileName); in = new BufferedReader(new InputStreamReader(_in,"UTF-8"));
       li.addFirst("ok");
       while ((inputLine = in.readLine()) != null) {li.add(inputLine);length+=inputLine.length(); }
     } catch (FileNotFoundException e) { e.printStackTrace();
       li.addFirst(e.getMessage());
     } catch (IOException e) {  e.printStackTrace();
     }
     finally{
       try {
         if (in!=null) in.close(); if (_in!=null) _in.close();
       } catch (IOException e) { e.printStackTrace(); }
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
      if (mode!=TERM_MODE) saveMessage("Added rule: ", user, ruleId, transport);
    }
    logger.error(e.getMessage());// e.printStackTrace();
    throw e;
   }
  }

  if (errorInfo == null || errorInfo.size() == 0) {
    if (mode!=TERM_MODE) saveMessage("Added rule: ", user, ruleId, transport);
  }
  else  {
    if (mode!=TERM_MODE) saveMessage("Failed to add rule: ", user, ruleId, transport);
    removeRuleFile(ruleId, transport);
  }
  return errorInfo;
}
  public synchronized LinkedList updateRule(BufferedReader r, final String ruleId, final String transport, int mode, String user) throws SibincoException,  IOException
 {
   //String ruleId=fileName.substring(5,fileName.length()-4);
   Rule rule = getRule(new Long(ruleId),transport);
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
         saveMessage("Updated rule: ", user, ruleId, transport);
         Functions.SavedFileToBackup(currentRuleFile,".new");
       }
     }
     else {
       if (mode != TERM_MODE) {
        saveMessage("Failed to update rule: ", user, ruleId, transport);
        saveRule(curbody,ruleId,rule.getTransport());
        currentRuleFile.delete();
       }
     }
     logger.error(e.getMessage());// e.printStackTrace();
     throw e;
    }
   }
    if (errorInfo == null || errorInfo.size()==0) {
        if (mode!=TERM_MODE) {
          saveMessage("Updated rule: ", user, ruleId, transport);
          Functions.SavedFileToBackup(currentRuleFile, ".new");
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

  private File saveCurrentRule(LinkedList li,String ruleId, String transport) {
    File ruleFile = composeRuleFile(transport,new Long(ruleId));
    File newFile = Functions.createNewFilenameForSave(ruleFile);
    try {
      PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), "UTF-8"));
      for (Iterator i = li.listIterator(1); i.hasNext();) {
          out.println(i.next());
      }
      out.flush();
      out.close();
    } catch (Exception e) {e.printStackTrace();}
    return newFile;
  }

  public void saveRule(LinkedList li,String ruleId, String transport) throws SibincoException
  {
    logger.debug("RESAVING current rule body to disc!!!!");

    try {
      File newFile= composeRuleFile(transport,new Long(ruleId));
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

  }

  private void saveRule(BufferedReader r,String ruleId, String transport) throws SibincoException
  {
      try {
        File newFile= composeRuleFile(transport,new Long(ruleId));
        logger.debug("Saving rule to disc!!!! file : " + newFile);
        final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), "UTF-8"));
        String s;
        while((s=r.readLine())!=null) {
          out.println(s);
        }
        out.flush();
        out.close();
      } catch (FileNotFoundException e) {
        throw new SibincoException("Couldn't save new rule : Couldn't write to destination config filename: " + e.getMessage());
      } catch (IOException e) {
        logger.error("Couldn't save new rule settings", e);
        throw new SibincoException("Couldn't save new rule template", e);
      }
    }

    public synchronized void removeRule(final String ruleId, final String transport, int mode, String user) throws SibincoException
    {
    //in term mode file deleted in removeRuleCommit(...) method in RuleManagerWrapper
          try {
            scag.removeRule(ruleId,transport);
            if (mode != TERM_MODE) {
              saveMessage("Removed rule: ", user, ruleId, transport);
              removeRuleFile(ruleId, transport);
            }
          } catch (SibincoException se) {
            if (se instanceof StatusDisconnectedException) {
              if (mode != TERM_MODE) {
               saveMessage("Removed rule: ", user, ruleId, transport);
               removeRuleFile(ruleId, transport);
              }
            }
            else {
              saveMessage("Failed to remove rule: ", user, ruleId, transport);
              throw se;
            }
          }
    }

    public void saveMessage(final String stringMessage, final String user, final String ruleId, final String transport) {
      StatMessage message = new StatMessage(user, "Rule", stringMessage + "rule_"+ ruleId+".xml for transport " + transport);
      StatusManager.getInstance().addStatMessages(message);
    }

    public void removeRulesForService(final String user,final String ruleId) throws SCAGJspException
    {
      try {
      String[] transports = Transport.transportTitles;
    for (byte i =0 ;i<transports.length;i++) {
           Rule current = getRule(new Long(ruleId),transports[i]);
           if (current!=null) removeRule(ruleId, current.getTransport(), NON_TERM_MODE, user);
        }
      } catch(SibincoException se) {
              se.printStackTrace();/*PRINT ERROR ON THE SCREEN;*/
              throw new SCAGJspException(ru.sibinco.scag.Constants.errors.rules.COULD_NOT_REMOVE_RULE, se);
       }
    }

    public void removeRuleFile(String ruleId, String transport) throws SibincoException
    {
        final File folder = new File(rulesFolder, transport);
        String filename="rule_"+ruleId+".xml";
        File fileForDeleting= new File(folder,filename);
        logger.debug("Trying to delete : " + fileForDeleting);
        if (fileForDeleting.exists())
          if (fileForDeleting.delete()!=true) throw new SibincoException("Couldn't delete rule " + fileForDeleting.getAbsolutePath());
    }

  public void unlockAllRules() {
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

