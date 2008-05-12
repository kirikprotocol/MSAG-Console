package ru.sibinco.smpp.ub_sme;

import org.apache.log4j.Category;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class ThreadConroller {
  private final static Category logger = Category.getInstance(ThreadConroller.class);

  private int threads;
  private int maxThreads;  

  public ThreadConroller(int maxThreads) {
    this.maxThreads = maxThreads;
  }

  public synchronized boolean start(){
    if(threads<maxThreads){
      threads++;
      return true;
    } else {
      return false;
    }
  }

  public synchronized void end(){
    threads--;
    this.notifyAll();
  }

}
