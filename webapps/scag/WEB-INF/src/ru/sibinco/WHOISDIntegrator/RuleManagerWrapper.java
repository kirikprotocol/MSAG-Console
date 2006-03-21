package ru.sibinco.WHOISDIntegrator;

import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;

import java.io.File;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 16.03.2006
 * Time: 11:36:21
 * To change this template use File | Settings | File Templates.
 */
public class RuleManagerWrapper {
  private final RuleManager rulemanager;
  public RuleManagerWrapper(RuleManager rulemanager) {
    this.rulemanager = rulemanager;
  }

  public LinkedList AddRule(Rule rule, final String ruleId) throws SibincoException, IOException {
  try {
    return rulemanager.AddRule(getRuleContent(getRuleContentAsString(rule)), ruleId, rule);
  } catch(SibincoException se) {
    if (se instanceof StatusDisconnectedException) {
     return null;
    } else {
      throw se;
    }
   }
  }

  public LinkedList updateRule(Rule rule, final String ruleId, final String transport) throws SibincoException, IOException {
  try {
    return rulemanager.updateRule(getRuleContent(getRuleContentAsString(rule)),ruleId,transport);
  } catch(SibincoException se) {
     if (se instanceof StatusDisconnectedException) {
     return null;
    } else {
      throw se;
    }
   }
  }

  public void removeRule(final String ruleId, final String transport) throws SibincoException {
      rulemanager.removeRule(ruleId, transport);
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
}
