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

import java.io.*;
import java.util.*;
import java.text.MessageFormat;


/**
 * Created by igork Date: 19.04.2004 Time: 19:05:23
 */
public class Rule
{
  private String transport;
  private Long id;
  private String notes;
  private LinkedList body = new LinkedList();
  public static String header;
  public static final LinkedList ending = new LinkedList();
  public static final String ROOT_ELEMENT="scag:rule";
  public static final String XML_LINE="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  private static final String XMLNS_XSI="http://www.w3.org/2001/XMLSchema-instance";
  private static final String XSI_SCHEMALOCATION="http://www.sibinco.com/SCAG ../xsd/";
  private static final String XMLNS_SCAG="http://www.sibinco.com/SCAG";
  private boolean locked = false;
  static {
       ending.add("</"+ROOT_ELEMENT+">");
  }

  public Rule(final Long id, final String notes, final String transport,final LinkedList body)
  {
    this.id=id;
    this.notes=notes;
    this.transport = transport;
    this.body.addAll(body);
  }

  public boolean isLocked() {
    return locked;
  }

  public void lock() {
    locked = true;
  }

  public void unlock() {
    locked = false;
  }

  public static Rule createNewRule(long id,String transport)
  {
    LinkedList body=new LinkedList();
    body.addAll(getRuleHeader(id,transport));
    body.addAll(ending);
    return new Rule(new Long(id),"",transport,body);
  }

 /* public static LinkedList getRuleHeader1(long id,String transport) {
    String schema=Transport.getSchemaByTransport(transport);
    LinkedList header=new LinkedList();
    header.add(XML_LINE);
    header.add(" <"+ROOT_ELEMENT+" xmlns:xsi=\""+XMLNS_XSI+"\"");
    header.add("           xsi:schemaLocation=\""+XSI_SCHEMALOCATION+schema+"\"");
    header.add("           xmlns:scag=\""+XMLNS_SCAG+"\"");
    header.add("           transport=\"" + transport + "\" id=\"" + id+"\">"+"\n");
    return header;
  } */

  public static LinkedList getRuleHeader(long id,String transport) {
    String schema=Transport.getSchemaByTransport(transport);
    LinkedList header= new LinkedList();
    String headerAsString = MessageFormat.format(Rule.header,new Object[]{schema, transport, new Long(id)} );
    BufferedReader br = new BufferedReader(new StringReader(headerAsString));
    String line;
    try {
    while((line=br.readLine())!=null) {
     header.add(line);
     //System.out.println(line);
     }
    } catch (IOException e) {
       e.printStackTrace();
    } finally {
      try {
      if (br!=null) br.close();
      } catch (IOException e) {e.printStackTrace();}
    }
    return header;
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

  public void appendBody(LinkedList body)
  {
    this.body.addAll(body);
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

