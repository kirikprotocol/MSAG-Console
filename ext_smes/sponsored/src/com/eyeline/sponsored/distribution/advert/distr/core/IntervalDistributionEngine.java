package com.eyeline.sponsored.distribution.advert.distr.core;

import org.apache.log4j.Category;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.sponsored.ds.distribution.advert.DistributionDataSource;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClient;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingException;
import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;

import java.util.*;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ThreadFactory;

import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 22.02.2008
 */

public class IntervalDistributionEngine implements DistributionEngine {
  private static final Category log = Category.getInstance("DISTRIBUTION");

  private final OutgoingQueue outQueue;
  private final DistributionDataSource distrDS;
  private final AdvertisingClient advClient;
  private final Map<String, DistributionInfo> distrInfos;
  private final ScheduledExecutorService executor;

  private long fetchInterval;
  private int sendSpeedLimit;
  private long prepareInterval;

  public IntervalDistributionEngine(OutgoingQueue outQueue, DistributionDataSource distrDS, AdvertisingClient advClient) {
    this.outQueue = outQueue;
    this.distrDS = distrDS;
    this.advClient = advClient;
    this.distrInfos = new HashMap<String, DistributionInfo>(10);
    this.executor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "IntDistrEngineThread");
      }
    });
  }

  public void init(long fetchInterval, long prepareInterval, int sendSpeedLimit) {
    this.fetchInterval = fetchInterval;
    this.prepareInterval = prepareInterval;
    this.sendSpeedLimit = sendSpeedLimit;
  }

  public void addDistribution(DistributionInfo distr) {
    this.distrInfos.put(distr.getDistributionName(), distr);
  }

  public void start() {
    final Work worker = new Work(fetchInterval, sendSpeedLimit);
    worker.setStartTime(System.currentTimeMillis() + prepareInterval);

    executor.scheduleAtFixedRate(worker, 0, fetchInterval, TimeUnit.MILLISECONDS);
  }

  public void stop() {
    executor.shutdown();
  }


  private class Work implements Runnable {

    private volatile long startTime;

    private final long fetchInterval;
    private final int sendSpeedLimit;

    public Work(long fetchInterval, int sendSpeedLimit) {
      this.fetchInterval = fetchInterval;
      this.sendSpeedLimit = sendSpeedLimit;
    }

    public void setStartTime(long startTime) {
      this.startTime = startTime;
    }

    private void sendDeliveries(List<Delivery> deliveries, Date end) {
      if (deliveries.isEmpty())
        return;

      final int shiftInterval = 1000 / sendSpeedLimit;

      int i = 0;
      long totalTime = 0;

      long curSendTime = 0;
      long curTimeShift = 0;

      for (Iterator<Delivery> iter = deliveries.iterator(); iter.hasNext(); i++) {
        Delivery d = iter.next();

        double endDate = d.getEndDate().getTime();
        double startDate = d.getStartDate().getTime();

        double sendInterval = (( endDate - startDate) / (d.getTotal() - 1));

        int msgNumber = (int)((end.getTime() - startDate) / sendInterval);

        long sendTime = Math.round(startDate + msgNumber * sendInterval);

        if (sendTime != curSendTime) {
          curSendTime = sendTime;
          curTimeShift = 0;
        } else {
          curTimeShift += shiftInterval;
        }

        try {
          // Lookup distribution info
          final DistributionInfo distr = distrInfos.get(d.getDistributionName());

          if (distr != null) {

            // Lookup banner
            long start = System.currentTimeMillis();
            final String banner = advClient.getBanner(distr.getAdvServiceName(), d.getSubscriberAddress());
            totalTime += System.currentTimeMillis() - start;

            if (banner != null) {

              // Send message
              final Message m = new Message();
              m.setSourceAddress(distr.getSrcAddress());
              m.setDestinationAddress(d.getSubscriberAddress());
              m.setMessageString(banner);
              m.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);
              final OutgoingObject o = new OutgoingObject();
              o.setMessage(m);

              try {
                if (log.isDebugEnabled()) {
                  log.debug("Send msg: distr=" + d.getDistributionName() + "; subscr=" + d.getSubscriberAddress() + "; msg=" + banner + "; time=" + new Date(sendTime + curTimeShift));
                }
                outQueue.offer(o, sendTime + curTimeShift);
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

        } catch (AdvertisingException e) {
          log.error("Get banner failed", e);
        }
      }

      if (log.isInfoEnabled())
        log.info("Get banner: total time=" + totalTime + "; average time=" + totalTime / i);

    }

    public void run() {
      List<Delivery> deliveries = null;
      try {
        final Date endDate = new Date(startTime + fetchInterval);

        long start = System.currentTimeMillis();
        deliveries = distrDS.lookupActiveDeliveries(new Date(startTime), endDate);
        if (log.isInfoEnabled())
          log.info("Deliveries fetch time: " + (System.currentTimeMillis() - start) + "; size=" + deliveries.size());

        // Process messages
        if (!deliveries.isEmpty()) {
          long startTime = System.currentTimeMillis();
          sendDeliveries(deliveries, endDate);
          if (log.isInfoEnabled())
            log.info("Send deliveries time=" + (System.currentTimeMillis() - startTime));
        }
        else if (log.isDebugEnabled())
          log.debug("Deliveries is empty: start=" + new Date(startTime) + "; end=" + endDate);

        startTime += fetchInterval;
      } catch (DataSourceException e) {
        log.error("Distribution failed.", e);
      } finally {
        if (deliveries != null)
          deliveries.clear();
      }
    }
  }
}
