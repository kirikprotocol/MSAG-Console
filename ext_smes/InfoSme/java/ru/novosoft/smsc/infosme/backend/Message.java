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
    private String userData = null;
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

    public String getUserData() {
        return userData;
    }

    public void setUserData(String userData) {
        if ( userData != null &&
             ( userData.indexOf('|') != -1 ||
               userData.indexOf(',') != -1 ||
             userData.indexOf('"') != -1 ||
             userData.indexOf('\\') != -1 ||
             userData.indexOf('\n') != -1 ) ) {
            throw new IllegalArgumentException("user data shall not contain chars: '|\\\",' and <CR>");
        }
        this.userData = userData;
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
    private final static State[] all = new State[7];

    public final static State UNDEFINED  = new State(-1, "UNDEFINED");
    public final static State NEW        = new State(0, "NEW");
    public final static State WAIT       = new State(1, "WAIT");
    public final static State ENROUTE    = new State(2, "ENROUTE");
    public final static State DELIVERED  = new State(3, "DELIVERED");
    public final static State EXPIRED    = new State(4, "EXPIRED");
    public final static State FAILED     = new State(5, "FAILED");
    public final static State DELETED    = new State(6, "DELETED");

    private final int id;
    private final String name;

    private State(int id, String name) {
      this.id = id;
      this.name = name;
      if (id >= 0)
        all[id] = this;
    }

    public int getId() {
      return id;  
    }

    public String getName() {
      return name;
    }

    public static State getById(int id) {
      return (id < 0 || id >= all.length) ? UNDEFINED : all[id];      
    }
  }
}
