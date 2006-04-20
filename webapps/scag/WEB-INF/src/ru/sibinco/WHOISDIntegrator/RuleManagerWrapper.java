package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.beans.rules.applet.MiscUtilities;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;

import java.io.File;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.ArrayList;
import java.lang.reflect.Method;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 16.03.2006
 * Time: 11:36:21
 * To change this template use File | Settings | File Templates.
 */
public class RuleManagerWrapper {
  public final static String TERM_COMMIT ="Commit";
  public final static String TERM_ROLLBACK ="Rollback";
  private final RuleManager rulemanager;
  private ArrayList ruleCommands;
  public RuleManagerWrapper(RuleManager rulemanager) {
    this.rulemanager = rulemanager;
    ruleCommands = new ArrayList(3);
  }

  public LinkedList AddRule(final Rule rule, final String service, final String transport) throws SibincoException, IOException {
  try {
    addRuleCommand("AddRule", service ,transport);
    return rulemanager.AddRule(getRuleContent(getRuleContentAsString(rule)), service, transport, RuleManager.TERM_MODE);
  } catch(SibincoException se) {
    if (se instanceof StatusDisconnectedException) {
     return null;
    } else {
      throw se;
    }
   }
  }

  public LinkedList updateRule(final Rule rule, final String service, final String transport) throws SibincoException, IOException {
  try {
    addRuleCommand("updateRule", service , transport);
    return rulemanager.updateRule(getRuleContent(getRuleContentAsString(rule)),service,transport, RuleManager.TERM_MODE);
  } catch(SibincoException se) {
     if (se instanceof StatusDisconnectedException) {
     return null;
    } else {
      throw se;
    }
   }
  }

  public void removeRule(final String service, final String transport) throws SibincoException {
      addRuleCommand("removeRule", service , transport);
      rulemanager.removeRule(service, transport, RuleManager.TERM_MODE);
  }

  public File getXslFolder() {
   return rulemanager.getXslFolder();
  }

  public Rule getRule(Long ruleId, String transport) {
    return rulemanager.getRule(ruleId,transport);
  }

  public String getRuleContentAsString(Rule rule){
    LinkedList ruleBody = rule.getBody();
    String ruleContent="";
    for(Iterator i = ruleBody.iterator();i.hasNext();) {
      ruleContent = ruleContent + i.next()+"\n";
    }
    return ruleContent;
  }

  private BufferedReader getRuleContent(String ruleContent){
    return new BufferedReader(new StringReader(ruleContent));
  }
  //--------------------------------
  public void AddRuleCommit(RuleCommand ruleCommand) {
    System.out.println("invoked AddRuleCommit service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    //do nothing!
  }

  public void updateRuleCommit(RuleCommand ruleCommand) throws Exception{
    System.out.println("invoked updateRuleCommit service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    //TODO
    //we have to save rule_1.xml.new in backup folder
    File ruleFile = rulemanager.composeRuleFile(ruleCommand.transport,new Long(ruleCommand.id));
    File currentRuleFile = Functions.createNewFilename(ruleFile);
    Functions.SavedFileToBackup(currentRuleFile,".new");
  }

  public void removeRuleCommit(RuleCommand ruleCommand) throws Exception {
    System.out.println("invoked removeRuleCommit service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    rulemanager.removeRuleFile(ruleCommand.id, ruleCommand.transport);
  }

  public void AddRuleRollback(RuleCommand ruleCommand) throws Exception{
    System.out.println("invoked AddRuleRollback service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    rulemanager.removeRuleFile(ruleCommand.id, ruleCommand.transport);
  }

  public void updateRuleRollback(RuleCommand ruleCommand) throws Exception {
    System.out.println("invoked updateRuleRollback service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    //TODO
    //rule_1.xml.new -> rule_1.xml and delete rule_1.xml.new
    File ruleFile = rulemanager.composeRuleFile(ruleCommand.transport,new Long(ruleCommand.id));
    File currentRuleFile = Functions.createNewFilename(ruleFile);
    if (!MiscUtilities.moveFile(currentRuleFile,ruleFile, true)) throw new Exception("Can't restore rule_"+ruleCommand.id+".xml from rule_"+ruleCommand.id+".xml.new");
  }

  public void removeRuleRollback(RuleCommand ruleCommand) {
    System.out.println("invoked removeRuleRollback service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    //do nothing
  }

  public void applyTerm(String prefix) throws Exception{
    for (Iterator i = ruleCommands.iterator(); i.hasNext();) {
      RuleCommand ruleCommand = (RuleCommand)i.next();
      Method command = this.getClass().getMethod(ruleCommand.commandName+prefix, new Class[]{RuleCommand.class});
      command.invoke(this,new Object[]{ruleCommand});
    }
  }

  public void addRuleCommand(String commandName, String id, String transport) {
    ruleCommands.add(new RuleCommand(commandName, id, transport));
  }

  private class RuleCommand {
    public String commandName;
    public String id;
    public String transport;
    RuleCommand(String commandName, String id, String transport) {
      this.commandName = commandName;
      this.id = id;
      this.transport = transport;
    }
  }
}
