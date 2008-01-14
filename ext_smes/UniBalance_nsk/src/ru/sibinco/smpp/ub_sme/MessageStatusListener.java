package ru.sibinco.smpp.ub_sme;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 6, 2006
 * Time: 1:01:37 PM
 */
public interface MessageStatusListener {
  public String getName();
  public void statusChanged(int messageId, int status);
}
