package ru.novosoft.smsc.infosme.backend;

import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2004
 * Time: 19:23:31
 * To change this template use Options | File Templates.
 */
public class Message
{
  public final static byte MESSAGE_UNDEFINED_STATE  = -1;
  public final static byte MESSAGE_NEW_STATE        =  0;
  public final static byte MESSAGE_WAIT_STATE       = 10;
  public final static byte MESSAGE_ENROUTE_STATE    = 20;
  public final static byte MESSAGE_DELIVERED_STATE  = 30;
  public final static byte MESSAGE_EXPIRED_STATE    = 40;
  public final static byte MESSAGE_FAILED_STATE     = 50;

  public final static String SORT_BY_DATE    = "date";
  public final static String SORT_BY_STATUS  = "status";
  public final static String SORT_BY_ABONENT = "abonent";

  public long id;
  public String abonent = null;
  public String message = null;
  public Date sendDate;
  public byte status = MESSAGE_UNDEFINED_STATE;

  public Message(long id, String abonent, String message, Date sendDate, byte status) {
    this.id = id; this.abonent = abonent; this.message = message;
    this.sendDate = sendDate; this.status = status;
  }
}
