package com.eyeline.sponsored.ds.banner;

/**
 * User: artem
 * Date: 11.04.2008
 */

public interface BannerMap {

  public void put(long messageId, int advertiserId);

  public int get(long messageId);

  public int size();

  public void close();
}
