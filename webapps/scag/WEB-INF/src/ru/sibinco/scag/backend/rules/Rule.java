package ru.sibinco.scag.backend.rules;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.08.2005
 * Time: 19:32:58
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.routing.TrafficRules;



/**
 * Created by igork Date: 19.04.2004 Time: 19:05:23
 */
public class Rule
{
  private static final String BILLING_ID_ATTRIBUTE = "billingId";
  private static final String PROVIDER_ID_ATTRIBUTE = "providerId";
  private Provider provider;
  private String billingRuleId;
  private String name;
  private String notes;

  public Rule(final String ruleName, final String notes, final Provider provider, final String billingRuleId)
  {
    this.name=ruleName;
    this.notes=notes;
    this.billingRuleId = billingRuleId;
    this.provider = provider;
  }

  public Provider getProvider()
  {
    return provider;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(String notes)
  {
    this.notes = notes;
  }

  public String getId()
   {
     return getName();
   }

  public void setId(String id)
  {
    setName(id);
  }

  public String getName()
  {
    return name;
  }

  public void setName(String ruleName)
  {
    this.name = ruleName;
  }

  public void setProvider(final Provider provider)
  {
    this.provider = provider;
  }

  public String getProviderName()
  {
    if (null != provider)
      return provider.getName();
    else
      return null;
  }


  public String getBillingRuleId()
  {
    return billingRuleId;
  }

  public void setBillingRuleId(String billingRuleId)
  {
    this.billingRuleId = billingRuleId;
  }
}

