package ru.sibinco.scag.backend.rules;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.08.2005
 * Time: 19:32:58
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.Constants;
import ru.sibinco.lib.backend.util.xml.Utils;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.Map;


/**
 * Created by igork Date: 19.04.2004 Time: 19:05:23
 */
public class Rule
{
  private static final String BILLING_ID_ATTRIBUTE = "billingId";
  private static final String PROVIDER_ID_ATTRIBUTE = "providerId";
  private Provider provider;
  private String transport;
  private String name;
  private String notes;

  public Rule(final String ruleName, final String notes, final Provider provider, final String transport)
  {
    this.name=ruleName;
    this.notes=notes;
    this.transport = transport;
    this.provider = provider;
  }
    public Rule(Element ruleElem) throws SibincoException
  {
    name = ruleElem.getAttribute("id");
    if (name.length() > Constants.ROUTE_ID_MAXLENGTH) {
      throw new SibincoException("Rule name is too long: " + name.length() + " chars \"" + name + '"');
    }
    transport=ruleElem.getAttribute("transport");
    //long providerId=Long.decode(ruleElem.getAttribute("provider")).longValue();
    NodeList providerList = ruleElem.getElementsByTagName("provider");
    if (providerList.getLength() > 1) {
          throw new SibincoException("Rule contains more then one provider");
        }
    long providerId=Long.decode(Utils.getNodeText(providerList.item(0))).longValue();
    notes = "";
    NodeList notesList = ruleElem.getElementsByTagName("notes");
    for (int i = 0; i < notesList.getLength(); i++)
      notes += Utils.getNodeText(notesList.item(i));
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


  public String getTransport()
  {
    return transport;
  }

  public void setTransport(String transport)
  {
    this.transport = transport;
  }
}

