package ru.sibinco.scag.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:55:21
 * To change this template use File | Settings | File Templates.
 */
public class ErrorCounterSet
{
  protected int errcode;
  protected long counter;

  ErrorCounterSet(int err, long count) {
    errcode = err; counter = count;
  }

  public int getErrcode() {
    return errcode;
  }

   protected void increment(long count)
  {
    this.counter += count;
  }
  public long getCounter() {
    return counter;
  }
}
