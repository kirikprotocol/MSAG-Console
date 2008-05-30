package ru.sibinco.smpp.ub_sme;

import ru.sibinco.util.threads.ThreadsPool;
import ru.aurorisoft.smpp.Message;

/**
 * Created by Serge Lugovoy
 * Date: Nov 22, 2007
 * Time: 3:32:27 PM
 */
public interface State {
  public Message getAbonentRequest();

  public void expire();

  public void startProcessing(ThreadsPool pool, SmeEngine smeEngine);

  public void closeProcessing(SmeEngine smeEngine);

  public void closeProcessing(ThreadsPool pool, SmeEngine smeEngine);

  public String getMessage();

  public void setMessage(String message);

  public boolean isExpired();

  public boolean isClosed();

  public Object getExpireObject();

}