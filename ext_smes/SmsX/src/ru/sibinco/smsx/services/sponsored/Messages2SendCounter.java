package ru.sibinco.smsx.services.sponsored;

/**
 * User: artem
 * Date: 12.03.2007
 */

public class Messages2SendCounter {
  private int counter = 0;

  public synchronized int getCounter() {
    return counter;
  }

  public synchronized void setCounter(int counter) {
    this.counter = counter;
    if (counter > 0)
      notify();
  }

  public synchronized void increase(long value) {
    counter+=value;
    notify();
  }

  public synchronized void decrease(long value) {
    counter-=value;
  }

  public synchronized void waitWhileCounterIsZero() {
    if (counter == 0)
      try {
        wait();
      } catch (InterruptedException e) {
      }
  }

  public synchronized void waiting(long interval) {
    try {
      wait(interval);
    } catch (InterruptedException e) {
    }
  }
}
