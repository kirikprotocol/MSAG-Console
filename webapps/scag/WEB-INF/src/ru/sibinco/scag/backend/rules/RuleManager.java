package ru.sibinco.scag.backend.rules;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;
import ru.sibinco.lib.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.backend.Gateway;

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
  private final Map subjects = Collections.synchronizedMap(new HashMap());
  private final Logger logger = Logger.getLogger(this.getClass());
  private final File rulesFolder;
  private final File xsdFolder;
  private long lastRuleId;
  private final ProviderManager providerManager;
  private final Gateway gateway;
  private static final String PARAM_NAME_LAST_USED_ID = "last used rule id";

  public RuleManager(final File rulesFolder,final File xsdFolder,final ProviderManager providerManager, final Config idsConfig,final Gateway gateway) throws Config.WrongParamTypeException, Config.ParamNotFoundException {

    this.lastRuleId = idsConfig.getInt(PARAM_NAME_LAST_USED_ID);
    this.rulesFolder = rulesFolder;
    this.xsdFolder = xsdFolder;
    this.providerManager = providerManager;
    this.gateway=gateway;
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

  /*  public Map getRuleMap(Long ruleId)
  {
    Rule rule= (Rule) rules.get(ruleId);
    return rule.getRuleMap();
  }  */

  public String getRuleTransportDir(String ruleId)
  {       Rule r=(Rule) rules.get(Long.decode(ruleId));
    return r.getTransport().toLowerCase();
  }

  public synchronized void load() throws SibincoException
  {
    try {
      loadFromFolder("smpp");
      loadFromFolder("http");
      loadFromFolder("mms");
    } catch (SibincoException e) {
      e.printStackTrace();
      System.out.println("error on init RuleManager in method load(): "+e.getMessage());
      throw new SibincoException(e.getMessage());
    }
  }

  public synchronized void loadFromFolder(String _transportDir) throws SibincoException
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
        String prov=el.getAttribute("provider");
        if (prov.equals(""))
          throw new SibincoException("Root element "+el.getTagName()+" not contain attribute 'provider'");
        Long providerId=Long.decode(prov);
        String name=el.getAttribute("name");
        if (name.equals(""))
          throw new SibincoException("Root element "+el.getTagName()+" not contain attribute 'name'");
        //NodeList child=el.getElementsByTagName("note");
        String notes="";
        /*  for (int j = 0; j < child.getLength(); j++) { Node node= child.item(j); notes+=Utils.getNodeText(node);
        }
        child=el.getElementsByTagName("provider");
         if (child.getLength() > 1) throw new SibincoException("Rule contains more then one provider");
          Node node = child.item(0); providerId=Long.decode(Utils.getNodeText(node));
        */
        Provider provider = (Provider) providerManager.getProviders().get(providerId);
        Long id=Long.decode(ruleId);

        rules.put(id, new Rule(id,name, notes,provider,transport,body/*,length*/));
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
      rules.put(id, new Rule(id,"unknown", notes,null,_transportDir.toLowerCase(),body/*,length*/));
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


 /* public synchronized void createNewRule(Rule newRule) throws SibincoException
  {
    saveNewRule(newRule);
  }

  private void saveNewRule(Rule rule) throws SibincoException
  {
    try {
      String filename="rule_"+String.valueOf(rule.getId())+".xml";
      String transport= rule.getTransport().toLowerCase();
      final File folder = new File(rulesFolder, transport);
      File newFile= new File(folder,filename);
      //final File newFile = Functions.createNewFilenameForSave(file);
      final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
      rule.storeTemplate(out);
      //Functions.storeConfigFooter(out, "scag:rules");
      out.flush();
      out.close();
      // Functions.renameNewSavedFileToOriginal(newFile, file);
    } catch (FileNotFoundException e) {
      throw new SibincoException("Couldn't save new rule : Couldn't write to destination config filename: " + e.getMessage());
    } catch (IOException e) {
      logger.error("Couldn't save new rule settings", e);
      throw new SibincoException("Couldn't save new rule template", e);
    }
  }
   */
 public synchronized void AddRule(BufferedReader r, final String ruleId,Rule newRule) throws SibincoException
{
  System.out.println("AddNewRule ruleId= "+ruleId);
  //gateway.addRule(ruleId);
  Long Id=Long.valueOf(ruleId);
  rules.put(Id,newRule);
  lastRuleId++;
  saveRule(r,ruleId);
}

  public synchronized LinkedList updateRule(BufferedReader r, final String ruleId) throws SibincoException
 {
   //String ruleId=fileName.substring(5,fileName.length()-4);
   System.out.println("updateRule ruleId= "+ruleId);
   LinkedList li=new LinkedList();
/*   try {
     li=(LinkedList) gateway.updateRule(ruleId);
   } catch (SibincoException e) {
     e.printStackTrace();//logger.warn(e.getMessage());  //To change body of catch statement use File | Settings | File Templates.

   } */
   saveRule(r,ruleId);
   return li;
 }

  private void saveRule(BufferedReader r,String ruleId) throws SibincoException
    {
      Rule rule=(Rule)rules.get(Long.valueOf(ruleId));
      try {
        String filename="rule_"+ruleId+".xml";
        String transport= rule.getTransport().toLowerCase();
        final File folder = new File(rulesFolder, transport);
        File newFile= new File(folder,filename);
        final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
        String s; LinkedList li=new LinkedList();li.addFirst("ok");
        while((s=r.readLine())!=null) { li.add(s);
          //System.out.println(s);
          out.println(s);
        }
        out.flush();
        out.close();
        //long length=newFile.length();
        rule.updateBody(li/*,length*/);
      } catch (FileNotFoundException e) {
        throw new SibincoException("Couldn't save new rule : Couldn't write to destination config filename: " + e.getMessage());
      } catch (IOException e) {
        logger.error("Couldn't save new rule settings", e);
        throw new SibincoException("Couldn't save new rule template", e);
      }
    }
    public synchronized void removeRule(String ruleId, String transport) throws SibincoException
    {
        //gateway.removeRule(ruleId);
          final File folder = new File(rulesFolder, transport);
          String filename="rule_"+ruleId+".xml";
          File fileForDeleting= new File(folder,filename);
          if (fileForDeleting.delete()!=true) throw new SibincoException("Couldn't delete rule " + fileForDeleting.getAbsolutePath());
    }

}

