package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import java.util.Date;
import java.util.TimeZone;
import java.util.Locale;
import java.util.GregorianCalendar;
import java.text.SimpleDateFormat;

public class SmsRow
{
  private static int numStates = 5;
  private static String states[] = {
    "ENROUTE", "DELIVERED", "EXPIRED", "UNDELIVERABLE", "DELETED"
  };

  private byte   id[];
  private Date   date = new Date();
  private String from = "from";
  private String to   = "to";
  private int status  = 0;
  private String text = "Message text ";
  private boolean marked = false;

  public void setId(byte _id[]) { id = _id; };
  public byte[] getId() { return id; };
  public String getIdString() {
    StringBuffer str = new StringBuffer();
    for (int i=0; i<id.length; i++) {
      int k = (((int)id[i])>>4)&0xf;
      str.append( (char)(k<10?k+48:k+55) );
      k = ((int)id[i])&0xf;
      str.append( (char)(k<10?k+48:k+55) );
    }
    return str.toString();
  };

  public String getFrom() { return from; }
  public void setFrom(String address) { from = address; }
  public String getTo(){ return to; };
  public void setTo(String address) { to = address; }
  public String getDateString()
  {
      SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
      return formatter.format(date);
  }
	public Date getDate()
	{
		return date;
	}
  public void setDate(Date date){ this.date = date; }
  public String getStatus(){
    return (status >= 0 && status < numStates) ? states[status]:"UNKNOUN";
  }
  public void setStatus(int status){ this.status = status; }
  public String getText(){ return text; }
  public void setText(String text){ this.text = text; }

  public boolean isMarked() { return marked; }
  public void mark() { marked = true; }
  public void unmark() { marked = false; }
};