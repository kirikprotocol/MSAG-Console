package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.util.Address;

/**
 * author: Aleksandr Khalitov
 */
public class Schedule {

  private Address subscriber;

  private long time;

  private byte eventsCount;

  private int lastError;

  public Address getSubscriber() {
    return subscriber;
  }

  public void setSubscriber(Address subscriber) {
    this.subscriber = subscriber;
  }

  public long getTime() {
    return time;
  }

  public void setTime(long time) {
    this.time = time;
  }

  public byte getEventsCount() {
    return eventsCount;
  }

  public void setEventsCount(byte eventsCount) {
    this.eventsCount = eventsCount;
  }

  public int getLastError() {
    return lastError;
  }

  public void setLastError(int lastError) {
    this.lastError = lastError;
  }
}
