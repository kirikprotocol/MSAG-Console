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

public class SmsRow
{
  private Date   date = new Date();
  private String from = "from";
  private String to   = "to";
  private int status  = 1010;
  private String text = "Message text ";

  public String getFrom() { return from; }
  public void setFrom(String address) { from = address; }
  public String getTo(){ return to; };
  public void setTo(String address) { to = address; }
  public String getDate(){ return ""+date.getTime(); }
  public void setDate(Date date){ this.date = date; }
  public int getStatus(){ return status; }
  public void setStatus(int status){ this.status = status; }
  public String getText(){ return text; }
  public void setText(String text){ this.text = text; }
};