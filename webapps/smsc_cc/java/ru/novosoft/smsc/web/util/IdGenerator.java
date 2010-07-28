package ru.novosoft.smsc.web.util;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * author: alkhal
 */
public class IdGenerator {

  private static AtomicInteger gen = new AtomicInteger();

  private String id;

  public IdGenerator() {
    this.id = "id_"+gen.incrementAndGet();
  }

  public String getId() {
    return id;
  }


  public String getIncrementAndGetId() {
    return id = "id_"+gen.incrementAndGet();
  }


}
