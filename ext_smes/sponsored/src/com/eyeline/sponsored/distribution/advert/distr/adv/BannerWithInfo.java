package com.eyeline.sponsored.distribution.advert.distr.adv;

/**
 * User: artem
 * Date: 16.04.2008
 */

public class BannerWithInfo {
  private String bannerText;
  private int advertiserId;

  public BannerWithInfo(String banner, int advertiserId) {
    this.bannerText = banner;
    this.advertiserId = advertiserId;
  }

  public String getBannerText() {
    return bannerText;
  }

  public int getAdvertiserId() {
    return advertiserId;
  }
}
