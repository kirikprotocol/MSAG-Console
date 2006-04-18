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
  private int whoisdPartOffset;
  private String transport;
  private Long id;
  private String notes;
  private LinkedList body = new LinkedList();
  public static String header;
  public static final LinkedList ending = new LinkedList();
  public static final String ROOT_ELEMENT="scag:rule";
  public static final String XML_LINE="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
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
    body.addAll(getRuleHeader(transport));
    body.addAll(ending);
    return new Rule(new Long(id),"",transport,body);
  }

  public static LinkedList getRuleHeader(String transport) {
    String schema=Transport.getSchemaByTransport(transport);
    LinkedList header= new LinkedList();
    String headerAsString = MessageFormat.format(Rule.header,new Object[]{schema, transport} );
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
    whoisdPartOffset = this.body.size();
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

  public int getWhoisdPartOffset()
  {
    return whoisdPartOffset;
  }

  public static String getRuleKey(Long id, String transport){
    return getRuleKey(id.toString(), transport);
  }

  public static String getRuleKey(String id, String transport){
    return transport+"/"+id;
  }

  public String getRuleKey() {
    return getRuleKey(id, transport);
  }
}

