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
  private String transport;
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


  public LinkedList getBody()
  {
    return body;
  }

  public void updateBody(LinkedList body)
  {
    this.body = body;
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

