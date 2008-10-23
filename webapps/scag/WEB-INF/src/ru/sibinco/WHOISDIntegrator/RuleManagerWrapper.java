package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.installation.HSDaemon;
import ru.sibinco.scag.beans.rules.applet.MiscUtilities;
import ru.sibinco.scag.beans.rules.RuleState;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;

import java.io.File;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.IOException;
import java.util.*;
import java.lang.reflect.Method;

import org.apache.log4j.Logger;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 16.03.2006
 * Time: 11:36:21
 * To change this template use File | Settings | File Templates.
 */
public class RuleManagerWrapper {
  protected final Logger logger = Logger.getLogger(this.getClass());

  public final static String TERM_COMMIT ="Commit";
  public final static String TERM_ROLLBACK ="Rollback";
  private final static String WHOISD_USER = "whoisd user";
  private final RuleManager rulemanager;
  private ArrayList ruleCommands;
  private ArrayList lockedRules;
  public RuleManagerWrapper(RuleManager rulemanager) {
    this.rulemanager = rulemanager;
    ruleCommands = new ArrayList(3);
    lockedRules = new ArrayList(3);
  }

  public LinkedList AddRule(final Rule rule, final String service, final String transport) throws SibincoException, IOException {
  try {
      logger.debug( "RuleManagerWrapper.AddRule() start" );
    addRuleCommand("AddRule", service ,transport);
      rulemanager.setSavePermissions( true );
      logger.debug( "RuleManagerWrapper.updateRule() return( AddRule(...) )" );
    return rulemanager.AddRule(getRuleContent(getRuleContentAsString(rule)), service, transport, RuleManager.TERM_MODE, WHOISD_USER);
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
    logger.debug( "RuleManagerWrapper.updateRule() start" );
    addRuleCommand("updateRule", service , transport);
    rulemanager.setSavePermissions( true );
    logger.debug( "RuleManagerWrapper.updateRule() return( updateRule(...) )" );
    return rulemanager.updateRule(getRuleContent(getRuleContentAsString(rule)),service,transport, RuleManager.TERM_MODE, WHOISD_USER);
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
      rulemanager.removeRule(service, transport, RuleManager.TERM_MODE, WHOISD_USER);
  }

  public File getXslFolder() {
   return rulemanager.getXslFolder();
  }

  public Rule getRule(String ruleId, String transport) {
    return rulemanager.getRule(ruleId,transport);
  }

  public RuleState getRuleStateAndLock(String ruleId, String transport) throws WHOISDException {
    RuleState curRuleState =  rulemanager.getRuleStateAndLock(ruleId,transport);
    if (curRuleState.getLocked()) throw new WHOISDException("Rule ["+transport+"] is editing in MSAG web admin console right now");
    lockedRules.add(Rule.composeComplexId(ruleId, transport));
    return curRuleState;
  }

  public void unlockRules() {
    for (Iterator i = lockedRules.iterator();i.hasNext(); ) {
      String complexRuleId = (String)i.next();
      rulemanager.unlockRule(complexRuleId);
    }
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
  public void AddRuleCommit(RuleCommand ruleCommand) throws Exception {
    logger.debug("invoked AddRuleCommit service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    rulemanager.finishOperation(ruleCommand.id,ruleCommand.transport,HSDaemon.UPDATEORADD);
    rulemanager.saveMessage("Added rule: ", WHOISD_USER, ruleCommand.id, ruleCommand.transport);
    //do nothing!
  }

  public void updateRuleCommit(RuleCommand ruleCommand) throws Exception {
    logger.debug("invoked updateRuleCommit service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    //we have to save rule_1.xml.new in backup folder
    File ruleFile = rulemanager.composeRuleFile(ruleCommand.transport,ruleCommand.id);
    File currentRuleFile = Functions.createNewFilename(ruleFile);
    Functions.SavedFileToBackup(currentRuleFile,".new");
    rulemanager.finishOperation(ruleCommand.id,ruleCommand.transport,HSDaemon.UPDATEORADD);
    rulemanager.saveMessage("Updated rule: ", WHOISD_USER, ruleCommand.id, ruleCommand.transport);
  }

  public void removeRuleCommit(RuleCommand ruleCommand) throws Exception {
    logger.debug("invoked removeRuleCommit service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    rulemanager.removeRuleFile(ruleCommand.id, ruleCommand.transport);
    rulemanager.finishOperation(ruleCommand.id,ruleCommand.transport,HSDaemon.REMOVE);
    rulemanager.saveMessage("Removed rule: ", WHOISD_USER, ruleCommand.id, ruleCommand.transport);
  }

  public void AddRuleRollback(RuleCommand ruleCommand) throws Exception{
    logger.debug("invoked AddRuleRollback service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    rulemanager.saveMessage("Failed to add rule: ", WHOISD_USER, ruleCommand.id, ruleCommand.transport);
    rulemanager.removeRuleFile(ruleCommand.id, ruleCommand.transport);
  }

  public void updateRuleRollback(RuleCommand ruleCommand) throws Exception {
    logger.debug("invoked updateRuleRollback service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    rulemanager.saveMessage("Failed to update rule: ", WHOISD_USER, ruleCommand.id, ruleCommand.transport);
    //rule_1.xml.new -> rule_1.xml and delete rule_1.xml.new
    File ruleFile = rulemanager.composeRuleFile(ruleCommand.transport,ruleCommand.id);
    File currentRuleFile = Functions.createNewFilename(ruleFile);
    if (!MiscUtilities.moveFile(currentRuleFile,ruleFile, true)) throw new Exception("Can't restore rule_"+ruleCommand.id+".xml from rule_"+ruleCommand.id+".xml.new");
  }

  public void removeRuleRollback(RuleCommand ruleCommand) {
    logger.debug("invoked removeRuleRollback service = "+ ruleCommand.id+" transport = "+ ruleCommand.transport);
    rulemanager.saveMessage("Failed to remove rule: ", WHOISD_USER, ruleCommand.id, ruleCommand.transport);
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

  public void clear() {
     ruleCommands.clear();
     lockedRules.clear();
  }

  private static class RuleCommand {
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
