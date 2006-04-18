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
  private final Map rules = Collections.synchronizedMap(new HashMap());
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


  private File composeRuleFile(String transport, Long serviceId) {
      String filename="rule_"+ serviceId.toString()+".xml";
      final File folder = new File(rulesFolder, transport);
      File newFile= new File(folder,filename);
      return newFile;
  }

  public Rule getRule(Long ruleId, String transport)
  {
    Rule rule = null;
    File ruleFile = composeRuleFile(transport, ruleId);
    rule = (Rule) rules.get(Rule.getRuleKey(ruleId,transport));
    if (rule!=null) {
      if (ruleFile.exists())
        rule.updateBody(LoadXml(ruleFile.getAbsolutePath()));
      else {
        rules.remove(rule);
        rule = null;
      }
    } else {
      if (ruleFile.exists()) {
         try {
         LoadRule(transport, ruleFile, rules);
         rule = (Rule) rules.get(Rule.getRuleKey(ruleId,transport));
         } catch (SibincoException se) {
           se.printStackTrace();
         }
      }
    }
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
      loadFromFolder(Transport.SMPP_TRANSPORT_NAME, rules);
      loadFromFolder(Transport.HTTP_TRANSPORT_NAME, rules);
      loadFromFolder(Transport.MMS_TRANSPORT_NAME, rules);
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

  public void loadFromFolder(String _transportDir, Map transportMap) throws SibincoException
  {
     File folder=new File(rulesFolder,_transportDir);
      if(!folder.exists())
        folder.mkdirs();
      File[] dir=folder.listFiles();
      for (int i = 0; i < dir.length; i++) {
        File file = dir[i];
        LoadRule(_transportDir, file, transportMap);
  }
 }

  private void LoadRule(String _transportDir, File file, Map transportMap) throws SibincoException {
    String fileName=file.getName();
    if (!fileName.endsWith(".xml")) return;

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

    Rule temp = new Rule(id,notes,transport,body);
    transportMap.put(temp.getRuleKey(),temp);
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
  Rule temp = new Rule(id, notes, _transportDir,body/*,length*/);
  transportMap.put(temp.getRuleKey(), temp);
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

  }

  private LinkedList LoadXml(final String fileName)
   {
     LinkedList li=new  LinkedList();
     InputStream _in=null; BufferedReader in = null; String inputLine;long length=0;
     try {  _in = new FileInputStream(fileName); in = new BufferedReader(new InputStreamReader(_in));
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

 public synchronized LinkedList AddRule(BufferedReader r, final String ruleId, final Rule newRule) throws SibincoException, IOException
 {
  String transport = newRule.getTransport();
  logger.debug("AddNewRule ruleId= "+ruleId + " ,transport = "+transport);
  LinkedList errorInfo=new LinkedList();
  Long Id=Long.valueOf(ruleId);

  //instead of scag.addRule(ruleId, transport);
  if (DEBUG) {
    String buffer = buffertostring(r);
    r = new BufferedReader(new StringReader(buffer));
    errorInfo = errorsImitator(r);
    r = new BufferedReader(new StringReader(buffer));
  }

  LinkedList newbody = saveRule(r,ruleId, newRule.getTransport());

  if (!DEBUG) {
  try {
    errorInfo =(LinkedList) scag.addRule(ruleId, transport);
  } catch (SibincoException e) {
    if (e instanceof StatusDisconnectedException) {
       newRule.updateBody(newbody);
       rules.put(newRule.getRuleKey(), newRule);
    } else {
      removeRuleFile(ruleId, newRule.getTransport());
    }
    logger.error(e.getMessage());// e.printStackTrace();
    throw e;
   }
  }

  if (errorInfo == null || errorInfo.size() == 0) {
    newRule.updateBody(newbody);
    rules.put(newRule.getRuleKey(), newRule);
  }
  else  {

    removeRuleFile(ruleId, newRule.getTransport());
  }
  return errorInfo;
}

  public synchronized LinkedList updateRule(BufferedReader r, final String ruleId, final String transport) throws SibincoException,  IOException
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
   File currentRuleFile = saveCurrentRule(curbody,ruleId, rule.getTransport());
   LinkedList newbody = saveRule(r,ruleId, rule.getTransport());

   if (!DEBUG)  {
   try    {
     errorInfo=(LinkedList) scag.updateRule(ruleId, transport);
   }    catch (SibincoException e)    {
     if (e instanceof StatusDisconnectedException) {
       rule.updateBody(newbody);
       Functions.SavedFileToBackup(currentRuleFile,".new");
     }
     else {
     saveRule(curbody,ruleId,rule.getTransport());
     currentRuleFile.delete();
     }
     logger.error(e.getMessage());// e.printStackTrace();
     throw e;
    }
   }
    if (errorInfo == null || errorInfo.size()==0) {
       rule.updateBody(newbody);
       Functions.SavedFileToBackup(currentRuleFile, ".new");
    } else {
       saveRule(curbody,ruleId,rule.getTransport());
       currentRuleFile.delete();
    }
   return errorInfo;
 }

  private File saveCurrentRule(LinkedList li,String ruleId, String transport) {
    File ruleFile = composeRuleFile(transport,new Long(ruleId));
    File newFile = Functions.createNewFilenameForSave(ruleFile);
    try {
      PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
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
      final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
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

  private LinkedList saveRule(BufferedReader r,String ruleId, String transport) throws SibincoException
  {
      LinkedList li=new LinkedList();
      try {
        File newFile= composeRuleFile(transport,new Long(ruleId));
        logger.debug("Saving rule to disc!!!! file : " + newFile);
        final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
        String s; li.addFirst("ok");
        while((s=r.readLine())!=null) { li.add(s);
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
      return li;
    }

    public synchronized void removeRule(final String ruleId, final String transport) throws SibincoException
    {
          try {
            scag.removeRule(ruleId,transport);
            rules.remove(Rule.getRuleKey(ruleId,transport));
            removeRuleFile(ruleId, transport);
          } catch (SibincoException se) {
            if (se instanceof StatusDisconnectedException) {
               rules.remove(Rule.getRuleKey(ruleId,transport));
               removeRuleFile(ruleId, transport);
            }
            else
              throw se;
          }
    }

    public void removeRulesForService(final String ruleId) throws SCAGJspException
    {
      try {
      String[] transports = Transport.transportTitles;
    for (byte i =0 ;i<transports.length;i++) {
           Rule current = (Rule)rules.get(Rule.getRuleKey(ruleId,transports[i]));
           if (current!=null) removeRule(ruleId, current.getTransport());
        }
      } catch(SibincoException se) {
              se.printStackTrace();/*PRINT ERROR ON THE SCREEN;*/
              throw new SCAGJspException(ru.sibinco.scag.Constants.errors.rules.COULD_NOT_REMOVE_RULE, se);
       }
    }

    private void removeRuleFile(String ruleId, String transport) throws SibincoException
    {
        final File folder = new File(rulesFolder, transport);
        String filename="rule_"+ruleId+".xml";
        File fileForDeleting= new File(folder,filename);
        logger.debug("Trying to delete : " + fileForDeleting);
        if (fileForDeleting.exists())
          if (fileForDeleting.delete()!=true) throw new SibincoException("Couldn't delete rule " + fileForDeleting.getAbsolutePath());
    }

  public void unlockAllRules() {
     for(Iterator i = rules.values().iterator();i.hasNext();) {
       Rule rule = (Rule)i.next();
       rule.unlock();
     }
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

