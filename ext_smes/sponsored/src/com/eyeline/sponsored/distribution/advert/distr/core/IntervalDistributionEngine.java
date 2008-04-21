package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClient;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingException;
import com.eyeline.sponsored.distribution.advert.distr.adv.BannerWithInfo;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.banner.BannerMap;
import com.eyeline.sponsored.ds.distribution.advert.DeliveriesDataSource;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SubmitResponse;

import java.util.*;
import java.util.concurrent.*;

/**
 * User: artem
 * Date: 22.02.2008
 */

public class IntervalDistributionEngine implements DistributionEngine {
  private static final Category log = Category.getInstance("DISTRIBUTION");

  private final OutgoingQueue outQueue;
  private final DeliveriesDataSource distrDS;
  private final AdvertisingClient advClient;
  private final Map<String, DistributionInfo> distrInfos;
  private final ScheduledExecutorService executor;
  private DeliveriesSender[] senders;
  private final BlockingQueue<Delivery> deliveriesQueue;
  private final BannerMap bannerMap;

  private long fetchInterval;
  private long prepareInterval;
  private Date end;

  public IntervalDistributionEngine(OutgoingQueue outQueue, DeliveriesDataSource distrDS, AdvertisingClient advClient, BannerMap bannerMap) {
    this.outQueue = outQueue;
    this.distrDS = distrDS;
    this.advClient = advClient;
    this.distrInfos = new HashMap<String, DistributionInfo>(10);
    this.bannerMap = bannerMap;
    this.deliveriesQueue = new ArrayBlockingQueue<Delivery>(100000);
    this.senders = null;
    this.executor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "IntDistrEngineThread");
      }
    });
  }

  public void init(long fetchInterval, long prepareInterval, int poolSize) {
    this.fetchInterval = fetchInterval;
    this.prepareInterval = prepareInterval;
    this.senders = new DeliveriesSender[poolSize];
    for (int i=0; i<senders.length; i++)
      senders[i] = new DeliveriesSender(i);
  }

  public void addDistribution(DistributionInfo distr) {
    this.distrInfos.put(distr.getDistributionName(), distr);
  }

  public void start() {
    final Work worker = new Work(fetchInterval);
    worker.setStartTime(System.currentTimeMillis() + prepareInterval);

    executor.scheduleAtFixedRate(worker, 0, fetchInterval, TimeUnit.MILLISECONDS);
    for(DeliveriesSender s : senders)
      s.start();
  }

  public void stop() {
    executor.shutdown();
    try {
      executor.awaitTermination(fetchInterval, TimeUnit.MILLISECONDS);
    } catch (InterruptedException e) {
      log.error(e,e);
    }
    for(DeliveriesSender s : senders)
      s.shuldown();
  }


  private class Work implements Runnable {

    private volatile long startTime;

    private final long fetchInterval;

    public Work(long fetchInterval) {
      this.fetchInterval = fetchInterval;
    }

    public void setStartTime(long startTime) {
      this.startTime = startTime;
    }

    public void run() {
      try {
        final Date endDate = new Date(startTime + fetchInterval);
        end = endDate;

        long start = System.currentTimeMillis();
        distrDS.lookupActiveDeliveries(new Date(startTime), endDate, deliveriesQueue);
        if (log.isInfoEnabled())
          log.info("Deliveries fetch time: " + (System.currentTimeMillis() - start));

        startTime += fetchInterval;
      } catch (DataSourceException e) {
        log.error("Distribution failed.", e);
      } catch (Throwable e) {
        log.error(e,e);
      }
    }
  }


  private class DeliveriesSender extends Thread {
    private boolean started = true;

    public DeliveriesSender(int number) {
      super("DeliveriesSender-" + number);
    }

    public void run() {
      int i = 0;
      long totalTime = 0;
      while(started) {
        try {

          Delivery d = deliveriesQueue.poll(10, TimeUnit.SECONDS);
          if (d == null)
            continue;

          i++;
          double endDate = d.getEndDate().getTime();
          double startDate = d.getStartDate().getTime();

          double sendInterval = (( endDate - startDate) / (d.getTotal() - 1));

          int msgNumber = (int)((end.getTime() - startDate) / sendInterval);

          long sendTime = Math.round(startDate + msgNumber * sendInterval);

          try {
            // Lookup distribution info
            final DistributionInfo distr = distrInfos.get(d.getDistributionName());

            if (distr != null) {

              // Lookup banner
              long start = System.currentTimeMillis();
              final BannerWithInfo banner = advClient.getBannerExt(distr.getAdvServiceName(), d.getSubscriberAddress());
              totalTime += System.currentTimeMillis() - start;

              if (banner != null) {

                // Send message
                final Message m = new Message();
                m.setSourceAddress(distr.getSrcAddress());
                m.setDestinationAddress(d.getSubscriberAddress());
                m.setMessageString(banner.getBannerText());
                m.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);
                final OutgoingObject o = new OutgoingObject() {
                  protected void handleResponse(PDU response) {
                    if (response.getStatusClass() == Message.STATUS_CLASS_NO_ERROR) {
                      try {
                        bannerMap.put(Long.parseLong(((SubmitResponse)response).getMessageId()), banner.getAdvertiserId());
                      } catch (NumberFormatException e) {
                        log.error(e,e);
                      }
                    }
                  }
                };
                o.setMessage(m);

                try {
                  if (log.isDebugEnabled())
                    log.debug("Send msg: distr=" + d.getDistributionName() + "; subscr=" + d.getSubscriberAddress() + "; msg=" + banner.getBannerText() + "; advId=" + banner.getAdvertiserId() + "; time=" + new Date(sendTime));

                  outQueue.offer(o, sendTime);
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

            if (i >= 1000) {
              if (log.isInfoEnabled())
                log.info("Get banner average time=" + totalTime/i);
              i = 0;
              totalTime = 0;
            }

          } catch (AdvertisingException e) {
            log.error("Get banner failed", e);
          }

        } catch (Throwable e) {
          log.error(e,e);
        }
      }
    }

    public void shuldown() {
      this.started = false;
    }
  }
}
