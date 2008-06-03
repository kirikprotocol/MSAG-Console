package ru.sibinco.smpp.ub_sme.mg;

import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.ub_sme.*;
import ru.sibinco.util.threads.ThreadsPool;

public interface MGState extends State {

  public static final byte UNKNOWN = 0;
  public static final byte MG_REQUEST_SENT = 1;
  public static final byte MG_ERROR = 2;
  public static final byte MG_OK = 3;

  public String getMGBalance();

  public void setMGBalance(String mgBalance);

  public int getMGState();

  public void setMGState(int mgState);

}