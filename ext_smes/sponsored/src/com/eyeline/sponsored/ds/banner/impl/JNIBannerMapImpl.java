package com.eyeline.sponsored.ds.banner.impl;

import com.eyeline.sponsored.ds.banner.BannerMap;
import com.eyeline.jstore.JStore4Java;

/**
 * User: artem
 * Date: 11.04.2008
 */

public final class JNIBannerMapImpl implements BannerMap {

  static {
    try {
      System.out.println("load library");
      System.loadLibrary("JStore4Java");
    } catch (UnsatisfiedLinkError e) {
      System.out.println("JStore4Java library not found");
      e.printStackTrace();
    }
  }

  final JStore4Java store;

  public JNIBannerMapImpl(String storeFile, int rollingTime, int maxCollizions) {
    this.store = new JStore4Java();
    this.store.Init(storeFile, rollingTime, maxCollizions);
  }

  public void put(long messageId, int advertiserId) {
    store.Insert(messageId, advertiserId);
  }

  public int get(long messageId) {
    int val[] = new int[1];
    if (store.Lookup(messageId, val)) {
      store.Delete(messageId);
      return val[0];
    }
    return -1;
  }
}
