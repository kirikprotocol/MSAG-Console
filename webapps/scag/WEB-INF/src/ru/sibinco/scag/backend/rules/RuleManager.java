package ru.sibinco.scag.backend.rules;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;

import java.util.Map;
import java.util.Collections;
import java.util.HashMap;
import java.io.*;

import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.Constants;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;

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
  private final ProviderManager providerManager;
  //private static final String SMSC_ROUTES_PRIMARY_CONFIG = "routes.xml";
  //private static final String SMSC_ROUTES_TEMPORAL_CONFIG = "routes_.xml";
  //private static final String SMSC_ROUTES_TRACEABLE_CONFIG = "routes__.xml";

  public RuleManager(final File rulesFolder,final File xsdFolder,final ProviderManager providerManager)
  {
    this.rulesFolder = rulesFolder;
    this.xsdFolder = xsdFolder;
    this.providerManager = providerManager;
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
  public Rule getRule(String ruleId)
  {
    return (Rule) rules.get(ruleId);
  }

  public String getRuleTransportDir(String ruleId)
  {       Rule r=(Rule) rules.get(ruleId);
    return r.getTransport().toLowerCase();
  }

  public synchronized void load() throws SibincoException
  {
    try {
    File smpp=new File(rulesFolder,"smpp");
    loadFromFolder(smpp);
    File wap= new File(rulesFolder,"wap");
    loadFromFolder(wap);
    File mms= new File(rulesFolder,"mms");

      loadFromFolder(mms);
    } catch (SibincoException e) {
      e.printStackTrace();//logger.warn(e.getMessage());  //To change body of catch statement use File | Settings | File Templates.
      System.out.println("error on init RuleManager in method load(): "+e.getMessage());
      throw new SibincoException(e.getMessage());
    }
  }

/*  public synchronized void trace() throws SibincoException
  {
    saveToFile(SMSC_ROUTES_TRACEABLE_CONFIG);
  }
  */
  public synchronized void loadFromFolder(final File folder) throws SibincoException
  {
    try {
      File[] dir=folder.listFiles();
      for (int i = 0; i < dir.length; i++) {
        File file = dir[i];
        String fileName=file.getName();
        logger.debug("enter " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
        System.out.println("FileName= "+fileName);
        final Document ruleDoc = Utils.parse(file.getAbsolutePath());
        Element el=ruleDoc.getDocumentElement();
        if (!el.getTagName().equals("scag:rule"))
           throw new SibincoException("Root element "+el.getTagName()+" not math with 'scag:rule'");
        String transport=el.getAttribute("transport");
        String id=el.getAttribute("id");
        String fileId=fileName.substring(5,fileName.length()-4);
        if (!fileId.equals(id))
          throw new SibincoException("FileId "+fileId+" in name of file : "+fileName+" do not math with rule id:"+id);
         if (id.length() > Constants.ROUTE_ID_MAXLENGTH)
          throw new SibincoException("Rule id is too long: " + id.length() + " chars \"" + id + '"');
        System.out.println("el.getNodeName()= "+el.getNodeName()+" el.getTagName()= "+el.getTagName());

        NodeList child=el.getElementsByTagName("note");
        Long providerId=null; String notes="";
        for (int j = 0; j < child.getLength(); j++) {
          Node node= child.item(j); notes+=Utils.getNodeText(node);
        }
        child=el.getElementsByTagName("provider");
         if (child.getLength() > 1)
          throw new SibincoException("Rule contains more then one provider");

          Node node = child.item(0);
          providerId=Long.decode(Utils.getNodeText(node));
        System.out.println("nl.getLength()= "+child.getLength());
        Provider provider = (Provider) providerManager.getProviders().get(providerId);

        rules.put(id, new Rule(id, notes,provider,transport));
        logger.debug("exit " + this.getClass().getName() + ".loadFromFile(\"" + fileName + "\")");
      }

    } catch (FactoryConfigurationError error) {
      logger.error("Couldn't configure xml parser factory", error);
      throw new SibincoException("Couldn't configure xml parser factory", error);
    } catch (ParserConfigurationException e) {
      logger.error("Couldn't configure xml parser", e);
      throw new SibincoException("Couldn't configure xml parser", e);
    } catch (SAXException e) {
      logger.error("Couldn't parse", e);
      throw new SibincoException("Couldn't parse", e);
    } catch (IOException e) {
      logger.error("Couldn't perfmon IO operation", e);
      throw new SibincoException("Couldn't perfmon IO operation", e);
    } catch (NullPointerException e) {
      logger.error("Couldn't parse", e);
      throw new SibincoException("Couldn't parse", e);
    }
  }
  /*
  private void saveToFile(final String filename) throws SibincoException
  {
    try {
      final File file = new File(rulesFolder, filename);
      final File newFile = Functions.createNewFilenameForSave(file);
      final PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
      Functions.storeConfigHeader(out, "routes", "routes.dtd", Functions.getLocaleEncoding());
      for (Iterator i = subjects.values().iterator(); i.hasNext();) {
        final Subject subject = (Subject) i.next();
        subject.store(out);
      }
      for (Iterator i = rules.values().iterator(); i.hasNext();) {
        final Route route = (Rule) i.next();
        route.store(out);
      }
      Functions.storeConfigFooter(out, "routes");
      out.flush();
      out.close();
      Functions.renameNewSavedFileToOriginal(newFile, file);
    } catch (FileNotFoundException e) {
      throw new SibincoException("Couldn't save new routes settings: Couldn't write to destination config filename: " + e.getMessage());
    } catch (IOException e) {
      logger.error("Couldn't save new routes settings", e);
      throw new SibincoException("Couldn't save new routes settings", e);
    }
  }

  private void loadRules() throws SibincoException
  {

    Provider provider=new Provider(1,"first provider");
    rules.put("rule_1", new Rule("rule_1","first  rule",provider,"SMPP"));
    rules.put("rule_2", new Rule("rule_2","second rule",provider,"WAP"));
    rules.put("rule_3", new Rule("rule_3","third  rule",provider,"MMS"));

  }
 protected Rule createRule(final String ruleName, final String notes, final Provider provider, final String transport) throws SibincoException
  {
    return new Rule(  ruleName,  notes,   provider,   transport);
  }  */





}

