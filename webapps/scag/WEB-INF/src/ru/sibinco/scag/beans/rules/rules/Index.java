package ru.sibinco.scag.beans.rules.rules;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.routing.GwRoute;
import ru.sibinco.lib.bean.TabledBean;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.08.2005
 * Time: 19:23:50
 * To change this template use File | Settings | File Templates.
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  private final Map rules = Collections.synchronizedMap(new HashMap());

  protected Collection getDataSource()
  {  // Provider provider=new Provider(1,"first");

    //return rules.values();
    Collection rules=appContext.getRuleManager().getRules().values();
     // rules.put("rule_1", new Rule("rule_1","first rule",provider,"BillingRuleid1"));
     // rules.put("rule_2", new Rule("rule_2","sekond rule",provider,"BillingRuleid1"));
     // rules.put("rule_3", new Rule("rule_3","third rule",provider,"BillingRuleid1"));

      return rules;
  }

  protected void delete()
  {
    rules.keySet().removeAll(checkedSet);
  }
}

