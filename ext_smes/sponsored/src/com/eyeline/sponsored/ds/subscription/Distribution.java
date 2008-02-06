package com.eyeline.sponsored.ds.subscription;

import com.eyeline.sponsored.ds.Storable;

/**
 *
 * @author artem
 */
public interface Distribution extends Storable {

  public String getName();

  public void setName(String name);

  public Status getStatus();

  public void setStatus(Status status);

  public enum Status {

    OPENED, CLOSED
  }
}
