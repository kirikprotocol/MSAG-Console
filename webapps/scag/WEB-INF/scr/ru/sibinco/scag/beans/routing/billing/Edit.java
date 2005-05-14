package ru.sibinco.smppgw.beans.routing.billing;

import ru.sibinco.smppgw.backend.routing.BillingRule;
import ru.sibinco.smppgw.beans.*;

import java.util.Map;


/**
 * Created by igork Date: 21.09.2004 Time: 19:32:02
 */
public class Edit extends EditBean
{
  private String id;
  private String rule;

  public String getId()
  {
    return id;
  }

  public void setId(String id)
  {
    this.id = id;
  }

  protected void load(final String loadId) throws SmppgwJspException
  {
    id = loadId;
    final BillingRule ruleObj = (BillingRule) appContext.getBillingManager().getRules().get(id);
    if (null != ruleObj)
      rule = ruleObj.getText();
    else
      rule = "";
  }

  protected void save() throws SmppgwJspException
  {
    final Map rules = appContext.getBillingManager().getRules();
    final BillingRule ruleObj = (BillingRule) rules.get(id);
    if (null != ruleObj)
      ruleObj.setText(rule);
    else
      rules.put(id, new BillingRule(id, rule));
    appContext.getStatuses().setBillingChanged(true);
    throw new DoneException();
  }

  public String getRule()
  {
    return rule;
  }

  public void setRule(String rule)
  {
    this.rule = rule;
  }
}
