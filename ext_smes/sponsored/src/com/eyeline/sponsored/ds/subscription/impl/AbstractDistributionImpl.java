package com.eyeline.sponsored.ds.subscription.impl;

import com.eyeline.sponsored.ds.subscription.Distribution;

/**
 *
 * @author artem
 */
public abstract class AbstractDistributionImpl implements Distribution {

  private String name;
  private Status status;
  
  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public Status getStatus() {
    return status;
  }

  public void setStatus(Status status) {
    this.status = status;
  }  

}
