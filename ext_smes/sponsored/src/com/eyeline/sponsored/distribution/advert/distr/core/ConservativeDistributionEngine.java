package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClient;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingException;
import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import com.eyeline.sponsored.ds.distribution.advert.DistributionDataSource;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.List;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class ConservativeDistributionEngine implements DistributionEngine {
  private static final Category log = Category.getInstance("DISTRIBUTION");

  private final OutgoingQueue outQueue;
  private final DistributionDataSource distrDS;  
  private final AdvertisingClient advClient;
  private final Map<String, DistributionInfo> distrInfos;

  private Work worker;
  private boolean started = true;


  public ConservativeDistributionEngine(OutgoingQueue outQueue, DistributionDataSource distrDS, AdvertisingClient advClient) {
    this.outQueue = outQueue;
    this.distrDS = distrDS;
    this.advClient = advClient;
    this.distrInfos = new HashMap<String, DistributionInfo>();
  }

  public void addDistribution(DistributionInfo distr) {
    this.distrInfos.put(distr.getDistributionName(), distr);
  }

  public void init(int limit, int interval) {
    this.worker = new Work(interval, limit);
  }

  public void start() {
    worker.start();
  }

  public void stop() {
    started = false;
    synchronized(worker) {
      worker.notifyAll();
    }
  }

  private class Work extends Thread {

    private final long fetchInterval;
    private final int sendSpeedLimit;

    public Work(long fetchInterval, int sendSpeedLimit) {
      super("ConsDistrEngineThread");
      this.fetchInterval = fetchInterval;
      this.sendSpeedLimit = sendSpeedLimit;
    }

    private void sendDeliveries(List<Delivery> deliveries, Date end, int totalLimit) {
      if (deliveries.size() == 0)
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
            final String banner = advClient.getBanner(distr.getAdvServiceName(), d.getSubscriberAddress());

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
                  log.debug("Send msg: distr=" + d.getDistributionName() + "; subscr=" + d.getSubscriberAddress() + "; msg=" + banner);
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

    public synchronized void run() {

      long endTime = System.currentTimeMillis() + fetchInterval;

      while(started) {

        try {
          final Date end = new Date(endTime);

          // Fetch active deliveries (deliveries with send date in [startTime, startTime + fetchInterval])
          final int totalLimit = (int)(sendSpeedLimit * fetchInterval / 1000);
          final List<Delivery> deliveries = distrDS.lookupActiveDeliveries(end, totalLimit);

          // Send message by every delivery but no more than totalLimit
          sendDeliveries(deliveries, end, totalLimit);

          // Wait next iteration (up to the end of current fetch interval)
          if (endTime - System.currentTimeMillis() > 0) {
            try {
              wait(endTime - System.currentTimeMillis());
            } catch (Throwable e) {
              log.error("Interrupted", e);
            }
          }

          endTime = endTime + fetchInterval;

        } catch (DataSourceException e) {
          log.error("Distribution failed.", e);
        }
      }
    }

  }
}
