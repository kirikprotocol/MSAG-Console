package com.eyeline.sponsored.ds.subscription.impl;

import com.eyeline.sponsored.ds.subscription.VolumeStat;

/**
 * User: artem
 * Date: 31.01.2008
 */

public class AbstractVolumeStatImpl implements VolumeStat {
  private int volume;
  private int numberOfSubscribers;

  public int getVolume() {
    return volume;
  }

  public void setVolume(int volume) {
    this.volume = volume;
  }

  public int getNumberOfSubscribers() {
    return numberOfSubscribers;
  }

  public void setNumberOfSubscribers(int numberOfSubscribers) {
    this.numberOfSubscribers = numberOfSubscribers;
  }
}
