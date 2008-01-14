package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.ProductivityControllable;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 11.12.2006
 * Time: 18:42:47
 * To change this template use File | Settings | File Templates.
 */
public class ProductivityControlObject implements ProductivityControllable {

  private long count;
  private boolean counterEnabled;
  private long counterStartTime;
  private String name;

  public ProductivityControlObject(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }

  public long getEventsCount() {
    return count;
  }

  public synchronized void count(){
    count++;
  }

  public void setEventsCounterEnabled(boolean enabled) {
    counterEnabled = enabled;
    if(counterEnabled){
      counterStartTime = System.currentTimeMillis();
    }
  }

  public synchronized void resetEventsCounter() {
    count = 0;
    counterStartTime = System.currentTimeMillis();
  }

  public long getCounterStartTime() {
    return counterStartTime;
  }

}
