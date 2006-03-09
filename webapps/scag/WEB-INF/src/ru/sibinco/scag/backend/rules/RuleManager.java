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
  private final Map smpprules = Collections.synchronizedMap(new HashMap());
  private final Map httprules = Collections.synchronizedMap(new HashMap());
  private final Map mmsrules = Collections.synchronizedMap(new HashMap());
  private final Map subjects = Collections.synchronizedMap(new HashMap());
  private final Logger logger = Logger.getLogger(this.getClass());
  private final File rulesFolder;
  private final File xsdFolder;
  private long lastRuleId;
  private final ProviderManager providerManager;
  private final Scag scag;
  private static final String PARAM_NAME_LAST_USED_ID = "last used rule id";
  private final static boolean DEBUG = false;
  public RuleManager(final File rulesFolder,final File xsdFolder,final ProviderManager providerManager, final Config idsConfig,final Scag scag) throws Config.WrongParamTypeException, Config.ParamNotFoundException {

    this.lastRuleId = idsConfig.getInt(PARAM_NAME_LAST_USED_ID);
    this.rulesFolder = rulesFolder;
    this.xsdFolder = xsdFolder;
    this.providerManager = providerManager;
    this.scag =scag;
  }

  public void init()
          throws SibincoException
  {
    load();
  }

  public Map getRules()
  {
    return rules;
  }

  public Map getRules(Long serviceId)
  {
    HashMap result = new HashMap();
    result.put(Transport.SMPP_TRANSPORT_NAME,smpprules.get(serviceId));
    result.put(Transport.HTTP_TRANSPORT_NAME,httprules.get(serviceId));
    result.put(Transport.MMS_TRANSPORT_NAME,mmsrules.get(serviceId));
    return result;
  }

  public Map getSubjects()
  {
    return subjects;
  }
  public Rule getRule(Long ruleId)
  {
    return (Rule) rules.get(ruleId);
  }
  public LinkedList getRuleBody(Long ruleId)
 {
   Rule rule= (Rule) rules.get(ruleId);
   return rule.getBody();
 }

  public Rule getRule(Long ruleId, String transport)
  {
    Rule rule = null;
    if (transport.equals(Transport.SMPP_TRANSPORT_NAME))
      rule = (Rule) smpprules.get(ruleId);
    else if (transport.equals(Transport.HTTP_TRANSPORT_NAME))
      rule = (Rule) httprules.get(ruleId);
    else if (transport.equals(Transport.MMS_TRANSPORT_NAME))
      rule = (Rule) mmsrules.get(ruleId);
    return rule;
  }

 public LinkedList getRuleBody(Long ruleId, String transport)
 {
   Rule rule = getRule(ruleId, transport);
   return rule.getBody();
 }

  public String getRuleTransportDir(String ruleId)
  {       Rule r=(Rule) rules.get(Long.decode(ruleId));
    return r.getTransport();
  }

  public synchronized void load() throws SibincoException
  {
    try {
      loadFromFolder(Transport.SMPP_TRANSPORT_NAME, smpprules);
      loadFromFolder(Transport.HTTP_TRANSPORT_NAME, httprules);
      loadFromFolder(Transport.MMS_TRANSPORT_NAME, mmsrules);
    } catch (SibincoException e) {
      e.printStackTrace();
      System.out.println("error on init RuleManager in method load(): "+e.getMessage());
      throw new SibincoException(e.getMessage());
    }
  }

  public synchronized void loadFromFolder(String _transportDir, Map transportMap) throws SibincoException
  {
     File folder=new File(rulesFolder,_transportDir);
      if(!folder.exists())
        folder.mkdirs();
      File[] dir=folder.listFiles();
      for (int i = 0; i < dir.length; i++) {
        File file = dir[i];
        String fileName=file.getName();
        if (!fileName.endsWith(".xml")) continue;

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
        String ruleId=el.getAttribute("id");
        if (ruleId.equals(""))
          throw new SibincoException("Root element "+el.getTagName()+" not contain attribute 'id'");
        String fileId=fileName.substring(5,fileName.length()-4);
        if (!fileId.equals(ruleId))
          throw new SibincoException("FileId "+fileId+" in name of file : "+fileName+" do not math with rule id:"+ruleId);
        if (ruleId.length() > Constants.ROUTE_ID_MAXLENGTH)
          throw new SibincoException("Rule id is too long: " + ruleId.length() + " chars \"" + ruleId + '"');
       //NodeList child=el.getElementsByTagName("note");
        String notes="";
        /*  for (int j = 0; j < child.getLength(); j++) { Node node= child.item(j); notes+=Utils.getNodeText(node);
        }
        child=el.getElementsByTagName("provider");
         if (child.getLength() > 1) throw new SibincoException("Rule contains more then one provider");
          Node node = child.item(0); providerId=Long.decode(Utils.getNodeText(node));
        */

        Long id=Long.decode(ruleId);

        Rule temp = new Rule(id,notes,transport,body/*,length*/);
        transportMap.put(id,temp);
        rules.put(id, temp);
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
      transportMap.put(id, temp);
      rules.put(id, temp);
      logger.error("Couldn't parse", e);
      System.out.println("RuleManager LoadFromFile fileName= "+fileName+" SAXException e= "+e.getMessage());
          // throw new SibincoException("Couldn't parse", e);
    } catch (IOException e) {
      logger.error("Couldn't perfmon IO operation", e);
      throw new SibincoException("Couldn't perfmon IO operation", e);
    } catch (NullPointerException e) {
      logger.error("Couldn't parse", e);
      throw new SibincoException("Couldn't parse", e);
    }
  }
 }

  public long getLastRuleId() {
    return lastRuleId;
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
     System.out.println("RuleManager.LoadXml length= "+length);
     return li;
   }


public synchronized LinkedList AddRule(BufferedReader r, final String ruleId,Rule newRule) throws SibincoException, IOException
{
  String transport = newRule.getTransport();
  System.out.println("AddNewRule ruleId= "+ruleId + " ,transport = "+transport);
  LinkedList errorInfo=new LinkedList();
  Long Id=Long.valueOf(ruleId);

  //instead of scag.addRule(ruleId, transport);
  if (DEBUG) {
    String buffer = buffertostring(r);
    r = new BufferedReader(new StringReader(buffer));
    errorInfo = errorsImitator(r);
    r = new BufferedReader(new StringReader(buffer));
  }

  LinkedList newbody = saveRule(r,ruleId, newRule);

  if (!DEBUG) {
  try {
    errorInfo =(LinkedList) scag.addRule(ruleId, transport);
  } catch (SibincoException e) {
    if (e instanceof StatusDisconnectedException) {
       newRule.updateBody(newbody);
       addRuleToMap(Id,transport,newRule);
       //rules.put(Id,newRule);
    } else {
      removeRuleFile(ruleId, newRule.getTransport());
    }
    e.printStackTrace();
    throw e;
   }
  }

  if (errorInfo == null || errorInfo.size() == 0) {
    newRule.updateBody(newbody);
    addRuleToMap(Id,transport,newRule);
  }
  else  {
    if (rules.remove(Id) != null) lastRuleId--;
    removeRuleFile(ruleId, newRule.getTransport());
  }
  return errorInfo;
}

  private void addRuleToMap(Long ruleId, String transport,Rule newRule) {
     if (transport.equals(Transport.SMPP_TRANSPORT_NAME))
        smpprules.put(ruleId, newRule);
    else if (transport.equals(Transport.HTTP_TRANSPORT_NAME))
        httprules.put(ruleId, newRule);
    else if (transport.equals(Transport.MMS_TRANSPORT_NAME))
        mmsrules.put(ruleId, newRule);
  }

  public synchronized LinkedList updateRule(BufferedReader r, final String ruleId, final String transport) throws SibincoException,  IOException
 {
   //String ruleId=fileName.substring(5,fileName.length()-4);
   Rule rule = getRule(new Long(ruleId),transport);
   System.out.println("updateRule ruleId= "+ruleId+" ,transport = "+transport);
   LinkedList errorInfo = new LinkedList();

   //instead of scag.updateRule(ruleId, transport);
   if (DEBUG) {
    String buffer = buffertostring(r);
    r = new BufferedReader(new StringReader(buffer));
    errorInfo = errorsImitator(r);
    r = new BufferedReader(new StringReader(buffer));
   }

   LinkedList curbody = rule.getBody();
   LinkedList newbody = saveRule(r,ruleId, rule);

   if (!DEBUG)  {
   try    {
     errorInfo=(LinkedList) scag.updateRule(ruleId, transport);
   }    catch (SibincoException e)    {
     if (e instanceof StatusDisconnectedException) {
       rule.updateBody(newbody);
     }
     else {
     saveRule(curbody,ruleId,rule);
     }
     e.printStackTrace();
     throw e;
    }
   }
    if (errorInfo == null || errorInfo.size()==0) {
       rule.updateBody(newbody);
    } else {
       saveRule(curbody,ruleId,rule);
    }
   return errorInfo;
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
  private void saveRule(LinkedList li,String ruleId, Rule rule) throws SibincoException
  {
    System.out.println("RESAVING current rule body to disc!!!!");
    //Rule rule=(Rule)rules.get(Long.valueOf(ruleId));

    try {
      String filename="rule_"+ruleId+".xml";
      String transport= rule.getTransport();
      final File folder = new File(rulesFolder, transport);
      File newFile= new File(folder,filename);
      final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
      for (Iterator i = li.listIterator(1); i.hasNext();)
        out.println(i.next());
      out.flush();
      out.close();
      //long length=newFile.length();
      //rule.updateBody(li/*,length*/);
    } catch (FileNotFoundException e) {
      throw new SibincoException("Couldn't save new rule : Couldn't write to destination config filename: " + e.getMessage());
    } catch (IOException e) {
      logger.error("Couldn't save new rule settings", e);
      throw new SibincoException("Couldn't save new rule template", e);
    }

  }

  private LinkedList saveRule(BufferedReader r,String ruleId, Rule rule) throws SibincoException
  {
      //if (rule == null) rule=(Rule)rules.get(Long.valueOf(ruleId));
      LinkedList li=new LinkedList();
      try {
        String filename="rule_"+ruleId+".xml";
        String transport= rule.getTransport();
        final File folder = new File(rulesFolder, transport);
        File newFile= new File(folder,filename);
        System.out.print("Saving rule to disc!!!! file : " + newFile);
        final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
        String s; li.addFirst("ok");
        while((s=r.readLine())!=null) { li.add(s);
          //System.out.println(s);
          out.println(s);
        }
        out.flush();
        out.close();
        //long length=newFile.length();
        //rule.updateBody(li/*,length*/);
      } catch (FileNotFoundException e) {
        throw new SibincoException("Couldn't save new rule : Couldn't write to destination config filename: " + e.getMessage());
      } catch (IOException e) {
        logger.error("Couldn't save new rule settings", e);
        throw new SibincoException("Couldn't save new rule template", e);
      }
      return li;
    }

    public synchronized void removeRule(String ruleId, String transport) throws SibincoException
    {
          try {
            scag.removeRule(ruleId,transport);
            removeRuleFromMap(ruleId, transport);
            removeRuleFile(ruleId, transport);
          } catch (SibincoException se) {
            if (se instanceof StatusDisconnectedException) {
               removeRuleFromMap(ruleId, transport);
               removeRuleFile(ruleId, transport);
            }
            else
              throw se;
          }
    }
    private void removeRuleFromMap(String ruleId, String transport)
    {
      if (transport.equals(Transport.SMPP_TRANSPORT_NAME))
         smpprules.remove(new Long(ruleId));
      else if (transport.equals(Transport.HTTP_TRANSPORT_NAME))
         httprules.remove(new Long(ruleId));
      else if (transport.equals(Transport.MMS_TRANSPORT_NAME))
         mmsrules.remove(new Long(ruleId));
    }
    private void removeRuleFile(String ruleId, String transport) throws SibincoException
    {
        final File folder = new File(rulesFolder, transport);
        String filename="rule_"+ruleId+".xml";
        File fileForDeleting= new File(folder,filename);
        System.out.println("Trying to delete : " + fileForDeleting);
        if (fileForDeleting.exists())
          if (fileForDeleting.delete()!=true) throw new SibincoException("Couldn't delete rule " + fileForDeleting.getAbsolutePath());
    }
}

