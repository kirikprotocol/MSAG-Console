package ru.novosoft.smsc.infosme.backend;

import java.util.Date;
import java.util.Map;
import java.util.HashMap;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2004
 * Time: 19:23:31
 * To change this template use Options | File Templates.
 */
public class Message
{
//  public final static byte MESSAGE_UNDEFINED_STATE  = -1;
//  public final static byte MESSAGE_NEW_STATE        =  0;
//  public final static byte MESSAGE_WAIT_STATE       = 10;
//  public final static byte MESSAGE_ENROUTE_STATE    = 20;
//  public final static byte MESSAGE_DELIVERED_STATE  = 30;
//  public final static byte MESSAGE_EXPIRED_STATE    = 40;
//  public final static byte MESSAGE_FAILED_STATE     = 50;

  public final static String SORT_BY_DATE    = "send_date";
  public final static String SORT_BY_STATUS  = "state";
  public final static String SORT_BY_ABONENT = "abonent";

  private String taskId;
  private String abonent = null;
  private String message = null;
  private Date sendDate = null;
  private State state = null;
//  public byte status = MESSAGE_UNDEFINED_STATE;

  public Message() {
  }

  public String getTaskId() {
    return taskId;
  }

  public void setTaskId(String taskId) {
    this.taskId = taskId;
  }

  public String getAbonent() {
    return abonent;
  }

  public void setAbonent(String abonent) {
    this.abonent = abonent;
  }

  public String getMessage() {
    return message;
  }

  public void setMessage(String message) {
    this.message = message;
  }

  public Date getSendDate() {
    return sendDate;
  }

  public void setSendDate(Date sendDate) {
    this.sendDate = sendDate;
  }

  public State getState() {
    return state;
  }

  public void setState(State state) {
    this.state = state;
  }

  public static final class State {
    private final static Map all = new HashMap(10);

    public final static State UNDEFINED  = new State(-1);
    public final static State NEW        = new State(0);
    public final static State WAIT       = new State(1);
    public final static State ENROUTE    = new State(2);
    public final static State DELIVERED  = new State(3);
    public final static State EXPIRED    = new State(4);
    public final static State FAILED     = new State(5);
    public final static State DELETED    = new State(6);

    private final int id;

    private State(int id) {
      this.id = id;
      all.put(new Integer(id), this);
    }

    public int getId() {
      return id;  
    }

    public static State getById(int id) {
      return (State)all.get(new Integer(id));
    }
  }
}
