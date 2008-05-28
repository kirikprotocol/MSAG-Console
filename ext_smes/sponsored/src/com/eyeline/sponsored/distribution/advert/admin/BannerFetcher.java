package com.eyeline.sponsored.distribution.advert.admin;

import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClient;
import com.eyeline.sponsored.distribution.advert.distr.adv.BannerWithInfo;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.Arguments;

import java.io.File;

/**
 * User: artem
 * Date: 27.05.2008
 */

public class BannerFetcher {
  public static void main(String[] args) {
    AdvertisingClient client = null;
    try {

      final Arguments a = new Arguments(args);

      final XmlConfig xmlConfig = new XmlConfig();
      xmlConfig.load(new File("conf/config.xml"));

      final Config conf = new Config(xmlConfig);

      client = new AdvertisingClient(conf.getAdvertisingHost(), conf.getAdvertisingPort(), conf.getAdvertisingConnTimeout());
      client.connect();

      BannerWithInfo banner = client.getBannerWithInfo(a.getStringAttr("-s"), a.getStringAttr("-a"));

      System.out.println("Result:");
      System.out.println(banner);

    } catch (Throwable e) {
      e.printStackTrace();;
    } finally {
      if (client != null)
        client.close();
    }
  }
}
