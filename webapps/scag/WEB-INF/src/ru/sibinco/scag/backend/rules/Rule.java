package ru.sibinco.scag.backend.rules;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.08.2005
 * Time: 19:32:58
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.sme.Provider;

import java.io.PrintWriter;


/**
 * Created by igork Date: 19.04.2004 Time: 19:05:23
 */
public class Rule
{
  private Provider provider;
  private String transport;
  private String name;
  private Long id;
  private String notes;

  public Rule(final Long id,final String ruleName, final String notes, final Provider provider, final String transport)
  {
    this.id=id;
    this.name=ruleName;
    this.notes=notes;
    this.transport = transport;
    this.provider = provider;
  }

    protected PrintWriter storeHeader(final PrintWriter out, final String transport,
                                      final String id, final String provider, final String name) {

        out.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
                "<scag:rule xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n" +
                "           xsi:schemaLocation=\"http://www.sibinco.com/SCAG xsd/smpp_rules.xsd\"\n" +
                "           xmlns:scag=\"http://www.sibinco.com/SCAG\"\n" +
                "           transport=\"" + transport + "\" id=\"" + id + "\" provider=\"" + provider
                + "\" name=\"" + name + "\">\n" + "\n" + "\n");

        return out;
    }

    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println();out.println();
        out.println("</scag:rule>");
        return out;
    }

    public PrintWriter storeTemplate(final PrintWriter out, final String transport,
                                      final String id, final String provider, final String name) {
        return storeFooter(storeHeader(out, transport, id, provider, name));
    }

  public PrintWriter storeTemplate(final PrintWriter out) {

      return storeFooter(storeHeader(out, transport, String.valueOf(id), provider.getName(), name));
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

  public Long getId()
   {
     return id;
   }
  public void setId(Long id)
    {
      this.id=id;
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

