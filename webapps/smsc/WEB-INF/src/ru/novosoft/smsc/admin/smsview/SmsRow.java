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
import java.text.SimpleDateFormat;

public class SmsRow
{
  private static int numStates = 5;
  private static String states[] = {
    "ENROUTE", "DELIVERED", "EXPIRED", "UNDELIVERABLE", "DELETED"
  };

  private Date   date = new Date();
  private String from = "from";
  private String to   = "to";
  private int status  = 0;
  private String text = "Message text ";

  public String getFrom() { return from; }
  public void setFrom(String address) { from = address; }
  public String getTo(){ return to; };
  public void setTo(String address) { to = address; }
  public String getDate() {
    SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
    return formatter.format(date);
  }
  public void setDate(Date date){ this.date = date; }
  public String getStatus(){
    return (status >= 0 && status < numStates) ? states[status]:"UNKNOUN";
  }
  public void setStatus(int status){ this.status = status; }
  public String getText(){ return text; }
  public void setText(String text){ this.text = text; }
};