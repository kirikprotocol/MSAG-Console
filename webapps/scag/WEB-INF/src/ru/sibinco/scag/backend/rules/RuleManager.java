package ru.sibinco.scag.backend.rules;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import java.util.Map;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.io.*;

import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.route.Route;
import ru.sibinco.lib.backend.route.Destination;
import ru.sibinco.lib.backend.route.Subject;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.routing.GwRoute;
import ru.sibinco.scag.backend.sme.Provider;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 19.08.2005
 * Time: 14:42:35
 * To change this template use File | Settings | File Templates.
 */

/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 15:49:22
 */


public class RuleManager
{
  private final Map rules = Collections.synchronizedMap(new HashMap());
  private final Map subjects = Collections.synchronizedMap(new HashMap());
  private final Logger logger = Logger.getLogger(this.getClass());

  private static final String SMSC_ROUTES_PRIMARY_CONFIG = "routes.xml";
  private static final String SMSC_ROUTES_TEMPORAL_CONFIG = "routes_.xml";
  private static final String SMSC_ROUTES_TRACEABLE_CONFIG = "routes__.xml";

  public RuleManager()
  {

  }

  public void init()
      throws SibincoException
  {
    loadRules();
  }

  public Map getRules()
  {
    return rules;
  }

  public Map getSubjects()
  {
    return subjects;
  }



  // rules.put("rule_1", new Rule("rule_1","first rule",provider,"BillingRuleid1"));
  // rules.put("rule_2", new Rule("rule_2","sekond rule",provider,"BillingRuleid1"));
  // rules.put("rule_3", new Rule("rule_3","third rule",provider,"BillingRuleid1"));


  private void loadRules() throws SibincoException
  {   Provider provider=new Provider(1,"first provider");
      rules.put("rule_1", createRule("rule_1","first  rule",provider,"BillingRuleid1"));
      rules.put("rule_2", createRule("rule_2","second rule",provider,"BillingRuleid1"));
      rules.put("rule_3", createRule("rule_3","third  rule",provider,"BillingRuleid1"));

  }

 protected Rule createRule(final String ruleName, final String notes, final Provider provider, final String billingRuleId) throws SibincoException
  {
    return new Rule(  ruleName,  notes,   provider,   billingRuleId);
  }





}

