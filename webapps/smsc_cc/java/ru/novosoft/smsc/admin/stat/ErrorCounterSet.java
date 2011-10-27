package ru.novosoft.smsc.admin.stat;

/**
 * author: Aleksandr Khalitov
 */
public class ErrorCounterSet {

  private final int errcode;

  private long counter;

  ErrorCounterSet(int err, long count) {
    errcode = err;
    counter = count;
  }

  void increment(long count) {
    this.counter += count;
  }

  public int getErrcode() {
    return errcode;
  }

  public long getCounter() {
    return counter;
  }
}
