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

import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

/**
 * User: artem
 * Date: 22.02.2008
 */

public class IntervalDistributionEngine implements DistributionEngine {
  private static final Category log = Category.getInstance("DISTRIBUTION");

  private final OutgoingQueue outQueue;
  private final DeliveriesDataSource distrDS;
  private final Map<String, DistributionInfo> distrInfos;
  private final BannerMap bannerMap;

  private final ScheduledExecutorService deliveriesFetcher;
  private final DeliveriesSender[] senders;
  private final DeliveriesQueue deliveriesQueue;

  private long fetchInterval;
  private long prepareInterval;

  private IntervalDistributionEngineMBean mbean;

  private volatile long deliveriesFetchTime;
  private volatile int overflows;
  private volatile int nullBannerCounter;

  public IntervalDistributionEngine(OutgoingQueue outQueue,
                                    DeliveriesDataSource distrDS,
                                    AdvertisingClientFactory advClientFactory,
                                    BannerMap bannerMap,
                                    long fetchInterval, long prepareInterval, int poolSize) {
    this.outQueue = outQueue;
    this.distrDS = distrDS;
    this.distrInfos = new HashMap<String, DistributionInfo>(10);
    this.bannerMap = bannerMap;

    this.deliveriesQueue = new DeliveriesQueue();
    this.fetchInterval = fetchInterval;
    this.prepareInterval = prepareInterval;

    this.senders = new DeliveriesSender[poolSize];
    for (int i=0; i<senders.length; i++)
      senders[i] = new DeliveriesSender(i, advClientFactory.createClient());

    this.deliveriesFetcher = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "IntDistrEngineThread");
      }
    });

  }

  public void addDistribution(DistributionInfo distr) {
    this.distrInfos.put(distr.getDistributionName(), distr);
  }

  public IntervalDistributionEngineMBean getMBean() {
    if (mbean == null)
      mbean = new IntervalDistributionEngineMBean(this);
    return mbean;
  }

  public int getOverflows() {
    return overflows;
  }

  public int getThreadsNumber() {
    return senders.length;
  }

  public int getDeliveriesQueueSize() {
    return deliveriesQueue.size();
  }

  public long getDeliveriesFetchTime() {
    return deliveriesFetchTime;
  }

  public int getNullBannerCounter() {
    return nullBannerCounter;
  }

  public void start() {
    deliveriesFetcher.scheduleAtFixedRate(new DeliveriesFetcherTask(), 0, fetchInterval, TimeUnit.MILLISECONDS);
    for(DeliveriesSender s : senders)
      s.start();
  }

  public void stop() {
    deliveriesFetcher.shutdown();
    try {
      deliveriesFetcher.awaitTermination(fetchInterval, TimeUnit.MILLISECONDS);
    } catch (InterruptedException e) {
      log.error(e,e);
    }
    for(DeliveriesSender s : senders)
      s.shutdown();
  }




  private class DeliveriesFetcherTask implements Runnable {

    private long fetchStartTime = System.currentTimeMillis();

    public void run() {
      try {
        final Date startDate = new Date(fetchStartTime + prepareInterval);
        final Date endDate = new Date(startDate.getTime() + fetchInterval);

        deliveriesQueue.setModificator(new DeliveryModificator() {
          public void modifyDelivery(Delivery d) {
            double endDateD = d.getEndDate().getTime();
            double startDateD = d.getStartDate().getTime();

            double sendInterval = ((endDateD - startDateD) / (d.getTotal() - 1));

            int msgNumber = (int)((endDate.getTime() - startDateD) / sendInterval);

            d.setSendDate(new Date(Math.round(startDateD + msgNumber * sendInterval)));
          }
        });

        if (log.isInfoEnabled())
          log.info("Fetch deliveries: from=" + startDate + "; to=" + endDate);
        
        if (!deliveriesQueue.isEmpty()) {
          log.error("Deliveries queue is not empty. But new deliveries loaded.");
          overflows++;

        } else {
          long startTime = System.nanoTime();
          distrDS.lookupDeliveries(startDate, endDate, deliveriesQueue);
          deliveriesFetchTime = System.nanoTime() - startTime;
          if (log.isInfoEnabled())
            log.info("Deliveries fetch time: " + deliveriesFetchTime + "; ~size=" + deliveriesQueue.size());
        }

        fetchStartTime += fetchInterval;

      } catch (DataSourceException e) {
        log.error("Fetch deliveries failed.", e);
      } catch (Throwable e) {
        log.error(e,e);
      }
    }
  }




  private final class DeliveriesSender extends Thread {

    private final AdvertisingClient advClient;
    private boolean started = true;

    public DeliveriesSender(int number, AdvertisingClient advClient) {
      super("DeliveriesSender-" + number);
      this.advClient = advClient;
    }

    public void run() {

      // Connect advertising client
      try {
        advClient.connect();
      } catch (Throwable e) {
        log.error(e,e);
      }

      while(started) {
        try {

          Delivery d = deliveriesQueue.poll(1, TimeUnit.SECONDS);
          if (d == null)
            continue;

          try {
            // Lookup distribution info
            final DistributionInfo distr = distrInfos.get(d.getDistributionName());

            if (distr != null) {

              // Lookup banner
              final BannerWithInfo banner = advClient.getBannerWithInfo(distr.getAdvServiceName(), d.getSubscriberAddress());

              if (banner != null) {

                // Send message
                final OutgoingObject o = new OutgoingObjectWithBanner(distr.getSrcAddress(), d.getSubscriberAddress(), bannerMap, banner);

                try {
                  if (log.isDebugEnabled())
                    log.debug("Send msg: distr=" + d.getDistributionName() + "; subscr=" + d.getSubscriberAddress() + "; msg=" + banner.getBannerText() + "; advId=" + banner.getAdvertiserId() + "; time=" + d.getSendDate());

                  if (!outQueue.offer(o, d.getSendDate().getTime()))
                    log.error("Send msg failed: Outgoing queue overflow.");
                    
                } catch (ShutdownedException e) {
                  log.error("Out queue shutdowned", e);
                  return;
                }

              } else {
                log.warn("Banner is null: subscr=" + d.getSubscriberAddress());
                nullBannerCounter++;
              }

            } else {
              log.warn("Distr is unknown: distr=" + d.getDistributionName() + "; subscr=" + d.getSubscriberAddress());
            }

          } catch (AdvertisingException e) {
            log.error("Get banner failed", e);
          }

        } catch (Throwable e) {
          log.error(e,e);
        }
      }

      // Close advertising client
      try {
        advClient.close();
      } catch (Throwable e) {
        log.error(e,e);
      }
    }

    public void shutdown() {
      this.started = false;
    }
  }
}
