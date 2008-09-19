package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClient;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClientFactory;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingException;
import com.eyeline.sponsored.distribution.advert.distr.adv.BannerWithInfo;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.banner.BannerMap;
import com.eyeline.sponsored.ds.distribution.advert.DeliveriesDataSource;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import org.apache.log4j.Category;

import javax.management.DynamicMBean;
import java.util.*;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class ConservativeDistributionEngine implements DistributionEngine {
  private static final Category log = Category.getInstance("DISTRIBUTION");

  private final OutgoingQueue outQueue;
  private final DeliveriesDataSource distrDS;
  private final BannerMap bannerMap;
  private final AdvertisingClient advClient;
  private final Map<String, DistributionInfo> distrInfos;

  private Work worker;


  public ConservativeDistributionEngine(OutgoingQueue outQueue, DeliveriesDataSource distrDS, AdvertisingClientFactory advClientFactory, BannerMap bannerMap) {
    this.outQueue = outQueue;
    this.distrDS = distrDS;
    this.bannerMap = bannerMap;
    this.advClient = advClientFactory.createClient();
    this.distrInfos = new HashMap<String, DistributionInfo>(50);
  }

  public void addDistribution(DistributionInfo distr) {
    this.distrInfos.put(distr.getDistributionName(), distr);
  }

  public DynamicMBean getMBean() {
    return null;
  }

  public void init(int limit, int interval) {
    this.worker = new Work(interval, limit);
  }

  public void start() {
    worker.start();
  }

  public void stop() {
    worker.shutdown();
  }

  private class Work extends Thread {

    private final long fetchInterval;
    private final int sendSpeedLimit;
    private boolean started = true;

    public Work(long fetchInterval, int sendSpeedLimit) {
      super("ConsDistrEngineThread");
      this.fetchInterval = fetchInterval;
      this.sendSpeedLimit = sendSpeedLimit;
    }

    private void sendDeliveries(List<Delivery> deliveries, Date end, int totalLimit) {
      if (deliveries.isEmpty())
        return;

      final long sendDelay = 1000 / sendSpeedLimit;
      final long startTime = System.currentTimeMillis();

      for (int i=0, j=0; i < totalLimit ; i++,j++) {

        if (j >= deliveries.size())
          j = 0;

        final Delivery d = deliveries.get(j);
        if (d.getSended() >= d.getTotal() || d.getSendDate().after(end))
          continue;

        try {
          // Lookup distribution info
          final DistributionInfo distr = distrInfos.get(d.getDistributionName());

          if (distr != null) {

            // Lookup banner
            final BannerWithInfo banner = advClient.getBannerWithInfo(distr.getAdvServiceName(), d.getSubscriberAddress());

            if (banner != null) {

              String advertiserName = distr.getAdvertiserName(banner.getAdvertiserId());
              String srcAddress = advertiserName == null ? distr.getSrcAddress() : advertiserName;

              String postfix = distr.getPostfix();
              if (postfix != null) {
                StringBuilder bannerText = new StringBuilder(banner.getBannerText());
                char lastChar = bannerText.charAt(bannerText.length() - 1);
                if (lastChar != '.' && lastChar != '!' && lastChar != '?')
                  bannerText.append('.');
                bannerText.append(postfix);
                banner.setBannerText(bannerText.toString());
              }

              // Send message
              final OutgoingObject o = new OutgoingObjectWithBanner(srcAddress, d.getSubscriberAddress(), bannerMap, banner);

              try {
                if (log.isDebugEnabled()) {
                  log.debug("Send msg: distr=" + d.getDistributionName() + "; subscr=" + d.getSubscriberAddress() + "; msg=" + banner.getBannerText());
                }
                outQueue.offer(o, startTime + sendDelay * i);
              } catch (ShutdownedException e) {
                log.error("Out queue shutdowned", e);
                return;
              }

            } else {
              log.warn("Banner is null: subscr=" + d.getSubscriberAddress());
            }

          } else {
            log.warn("Distr is unknown: distr=" + d.getDistributionName() + "; subscr=" + d.getSubscriberAddress());
          }

          // Increase sended counter
          d.setSended(d.getSended() + 1);

          // If delivery contains more messages, calculate next send date
          if (d.getSended() < d.getTotal()) {
            final long nextSendDate =d.getSendDate().getTime() + (d.getEndDate().getTime() - d.getSendDate().getTime()) / (d.getTotal() - d.getSended());
            d.setSendDate(new Date(nextSendDate));
          }

          // Update delivery
          try {
            d.update();
          } catch (DataSourceException e) {
            log.error("Delivery update failed", e);
          }

        } catch (AdvertisingException e) {
          log.error("Get banner failed", e);
        }

      }
    }

    public void shutdown() {
      started = false;
    }

    public void run() {

      try {
        advClient.connect();
      } catch (Throwable e) {
        log.error(e,e);
      }

      long endTime = System.currentTimeMillis() + fetchInterval;
      final Object o = new Object();

      while(started) {

        try {
          final Date end = new Date(endTime);

          // Fetch active deliveries (deliveries with send date in [startTime, startTime + fetchInterval])
          final int totalLimit = (int)(sendSpeedLimit * fetchInterval / 1000);
          final List<Delivery> deliveries = new LinkedList<Delivery>();
          distrDS.lookupDeliveries(end, totalLimit, deliveries);

          // Send message by every delivery but no more than totalLimit
          sendDeliveries(deliveries, end, totalLimit);

          // Wait next iteration (up to the end of current fetch interval)

          long waitInterval = endTime - System.currentTimeMillis();
          if (waitInterval > 0) {
            synchronized(o) {
              try {
                o.wait(waitInterval);
              } catch (Throwable e) {
                log.error("Interrupted", e);
              }
            }
          }

          endTime = endTime + fetchInterval;

        } catch (DataSourceException e) {
          log.error("Distribution failed.", e);
        }
      }

      try {
        advClient.close();
      } catch (Throwable e) {
        log.error(e,e);
      }
    }

  }
}
