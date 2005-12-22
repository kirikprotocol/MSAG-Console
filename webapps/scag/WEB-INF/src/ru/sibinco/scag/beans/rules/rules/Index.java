package ru.sibinco.scag.beans.rules.rules;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.lib.SibincoException;

import javax.servlet.http.HttpSession;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
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
  //private final Map rules = Collections.synchronizedMap(new HashMap());
  private boolean newRule=false;
  protected HttpSession session = null;

  public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException
   {
     session = request.getSession(false);
     super.process(request, response);
   }
  protected Collection getDataSource()
  {
     RuleManager ruleManager=appContext.getRuleManager();
     newRule=(session.getAttribute("newRule")!=null);
     return ruleManager.getRules().values();
  }

  public boolean isNewRule()
  {
    return newRule;
  }

  protected void delete()
  {
    RuleManager ruleManager=appContext.getRuleManager();
    final Map rules =  ruleManager.getRules();
    //rules.keySet().removeAll(checkedSet);
    for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();)
    {
          final String ruleId = (String) iterator.next();
          Rule rule = (Rule)rules.get(Long.decode(ruleId));
          try
          {
            ruleManager.removeRule(ruleId, rule.getTransport().toLowerCase());
          }
          catch (SibincoException e)
          {
              e.printStackTrace();
          }
          rules.remove(Long.decode(ruleId));
    }
  }
}

