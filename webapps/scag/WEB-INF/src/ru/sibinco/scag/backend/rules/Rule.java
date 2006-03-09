package ru.sibinco.scag.backend.rules;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.08.2005
 * Time: 19:32:58
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.transport.Transport;

import java.io.PrintWriter;
import java.util.LinkedList;
import java.util.Map;
import java.util.Collections;
import java.util.HashMap;


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
  private LinkedList body;
  public  static final String ROOT_ELEMENT="scag:rule";
  private static final String XML_LINE="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  private static final String XMLNS_XSI="http://www.w3.org/2001/XMLSchema-instance";
  private static final String XSI_SCHEMALOCATION="http://www.sibinco.com/SCAG xsd/";
  private static final String XMLNS_SCAG="http://www.sibinco.com/SCAG";

  public Rule(final Long id, final String notes, final String transport,final LinkedList body)
  {
    this.id=id;
    this.notes=notes;
    this.transport = transport;
    this.body =body;
  }

  public static Rule createNewRule(long id,String transport)
  {
    String schema=Transport.getSchemaByTransport(transport);
    LinkedList body=new LinkedList();
    body.add(XML_LINE);
    body.add("<"+ROOT_ELEMENT+" xmlns:xsi=\""+XMLNS_XSI+"\"");
    body.add("           xsi:schemaLocation=\""+XSI_SCHEMALOCATION+schema+"\"");
    body.add("           xmlns:scag=\""+XMLNS_SCAG+"\"");
    body.add("           transport=\"" + transport + "\" id=\"" + id+"\">"+"\n"+"\n");
    body.add("</"+ROOT_ELEMENT+">");
    return new Rule(new Long(id),"",transport,body);
  }

/*  public static Rule createNewRule(long id,String transport,Provider provider,String name) {
    String lineSeparator=System.getProperty("line.separator");
    int sep=lineSeparator.length();
    String schema=Transport.getSchemaByTransport(transport);
    System.out.println("XML_LINE length= "+XML_LINE.length());
    LinkedList body=new LinkedList();
    long length=0;

    String Line1="<"+ROOT_ELEMENT+" xmlns:xsi=\""+XMLNS_XSI+"\"";
    String Line2="           xsi:schemaLocation=\""+XSI_SCHEMALOCATION+schema+"\"";
    String Line3="           xmlns:scag=\""+XMLNS_SCAG+"\"";
    String Line4="           transport=\"" + transport + "\" id=\"" + id + "\" provider=\"" + provider.getId()
                + "\" name=\"" + name + "\">"+"\n"+"\n";
    String Line5="</"+ROOT_ELEMENT+">";
    length+=XML_LINE.length();length+=Line1.length();length+=Line2.length();
    length+=Line3.length();length+=Line4.length();length+=Line5.length();length+=sep*6;
    body.add(XML_LINE);body.add(Line1);body.add(Line2);body.add(Line3);body.add(Line4);body.add(Line5);
    return new Rule(new Long(id),name,"",provider,transport,body,length);
  } */
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

 /* public Map getRuleMap()
  {
    return ruleMap;
  }
   */
  public LinkedList getBody()
  {
    return body;
  }

  public void updateBody(LinkedList body/*,long length*/)
  {
    this.body = body;
   /* this.length=length;
    ruleMap.put("body",body);
    ruleMap.put("length",new Long(length));
  */
  }

/*  public long getLength()
  {
    return length;
  }
  */
  public String getTransport()
  {
    return transport;
  }

  public void setTransport(String transport)
  {
    this.transport = transport;
  }

}

