package ru.sibinco.smppgw.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:55:21
 * To change this template use File | Settings | File Templates.
 */
public class ErrorCounterSet
{
  public int errcode;
  public int counter;

  ErrorCounterSet(int err, int count) {
    errcode = err; counter = count;
  }

  public int getErrcode() {
    return errcode;
  }
  public int getCounter() {
    return counter;
  }
}
