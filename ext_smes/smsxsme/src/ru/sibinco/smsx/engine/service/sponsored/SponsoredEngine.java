package ru.sibinco.smsx.engine.service.sponsored;

import com.eyeline.sme.utils.ds.DataSourceException;
import com.eyeline.sme.utils.worker.IterativeWorker;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.advertising.AdvertisingClientException;
import ru.sibinco.smsx.network.advertising.AdvertisingClientFactory;
import ru.sibinco.smsx.engine.service.sponsored.datasource.SponsoredDataSource;
import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

import java.util.*;

/**
 * User: artem
 * Date: 29.06.2007
 */

class SponsoredEngine {

  private static final Category log = Category.getInstance("SPONSORED");

  private static final int ABONENTS_PORTION = 1000;

  // Properties
  private String advertisingClientName;
  private int fromHour;
  private int toHour;
  private String smeAddress;

  private final SMPPOutgoingQueue outQueue;
  private final SponsoredDataSource ds;
  private final AdvertisingClient advertisingClient;
  private final List groupWorkers;


  SponsoredEngine(final SMPPOutgoingQueue outQueue, SponsoredDataSource ds, int[] allowedCounts) {
    this.outQueue = outQueue;
    this.ds = ds;

    this.advertisingClient = AdvertisingClientFactory.createAdvertisingClient();
    this.advertisingClient.connect();

    groupWorkers = new LinkedList();
    try {
      for (int i = 0; i < allowedCounts.length; i++)
        groupWorkers.add(new AbonentsGroupWorker(allowedCounts[i]));


    } catch (Throwable e) {
      throw new InitializationException(e);
    }
  }

  public void setAdvertisingClientName(String advertisingClientName) {
    this.advertisingClientName = advertisingClientName;
  }

  public void setFromHour(int fromHour) {
    this.fromHour = fromHour;
  }

  public void setToHour(int toHour) {
    this.toHour = toHour;
  }

  public void setSmeAddress(String smeAddress) {
    this.smeAddress = smeAddress;
  }

  public void startService() {
    for (Iterator iter = groupWorkers.iterator(); iter.hasNext();)
      ((AbonentsGroupWorker)iter.next()).start();
  }

  public void stopService() {
    for (Iterator iter = groupWorkers.iterator(); iter.hasNext();)
      ((AbonentsGroupWorker)iter.next()).stop();

    advertisingClient.close();
  }





  private class AbonentsGroupWorker extends IterativeWorker {

    private final Object startDayWaiter = new Object();
    private final Object waitAbonentsObject = new Object();
    private final Object delayObject = new Object();

    private final int subscriptionCount;

    private AbonentsGroupWorker(int subscriptionCount) {
      super(log);
      this.subscriptionCount = subscriptionCount;
    }

    public void iterativeWork() {
      // Calculate day start
      final Calendar dayStart = Calendar.getInstance();
      dayStart.setTime(new Date());

      if (dayStart.get(Calendar.HOUR_OF_DAY) >= toHour)
        dayStart.setTimeInMillis(dayStart.getTimeInMillis() + 24*3600000); // Add one day

      dayStart.set(Calendar.HOUR_OF_DAY, fromHour);
      dayStart.set(Calendar.MINUTE, 0);
      dayStart.set(Calendar.SECOND, 0);
      dayStart.set(Calendar.MILLISECOND, 0);

      // Waiting for day startService
      long timeout = dayStart.getTimeInMillis() - System.currentTimeMillis();
      if (timeout > 0) {
        log.debug("Waiting for day startService: " + timeout);
        synchronized(startDayWaiter) {
          try {
            startDayWaiter.wait(timeout);
          } catch (InterruptedException e) {
            log.error("Interrupted:", e);
          }
        }
      }

      // Calculate day end
      final Calendar endTime = Calendar.getInstance();
      endTime.setTime(new Date());
      endTime.set(Calendar.HOUR_OF_DAY, toHour);
      endTime.set(Calendar.MINUTE, 0);
      endTime.set(Calendar.SECOND, 0);
      endTime.set(Calendar.MILLISECOND, 0);

      final long endTimeLong = endTime.getTimeInMillis();

      // Calculate total abonents count (if total abonents == 0 wait)
      int totalAbonentsCount = waitAbonents(endTime.getTimeInMillis());

      long iterStartTime, totalMessagesCount, delay;
      double expectedDelay, inaccurancy = 0, totalInaccurancy = 0;
      String abonent, banner;

      while (totalAbonentsCount > 0 && isStarted()) {

        // Calculate total messages count
        totalMessagesCount = getTotalMessagesCount();
        System.out.println("Total count = " + totalMessagesCount);
        System.out.println("Average speed: " + totalMessagesCount * 1000 / (endTime.getTimeInMillis() - System.currentTimeMillis()));

        // Send one message to each abonent
        for (int j=0; j < totalAbonentsCount && isStarted(); j+=ABONENTS_PORTION) {

          try {
            // Get next abonents list
            final Collection abonents = ds.getAbonents(subscriptionCount, j, ABONENTS_PORTION);

            for (Iterator iter = abonents.iterator(); iter.hasNext() && isStarted();) {

              // start of new iteraction
              iterStartTime = System.currentTimeMillis();

              // calculate initial wait time
              expectedDelay = (double)(endTimeLong - iterStartTime) / (double)totalMessagesCount - inaccurancy;

              abonent = (String)iter.next();

              // get banner
              banner = getBannerForAbonent(abonent);

              // decrease counter for this abonent
              decreaseMessagesCount(abonent);

              // decrease tick counter
              totalMessagesCount--;

              // prepare smpp message
              final Message message = new Message();
              message.setSourceAddress(smeAddress);
              message.setDestinationAddress(abonent);
              message.setMessageString(banner);
              message.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);
              final SMPPTransportObject outObj = new SMPPTransportObject();
              outObj.setOutgoingMessage(message);

              // Calculate time to wait
              delay = (long)(expectedDelay - (System.currentTimeMillis() - iterStartTime));
              if (delay > 1 && totalInaccurancy > 1000) {
                synchronized(delayObject) {
                  try {
                    delayObject.wait(delay);
                  } catch (InterruptedException e) {
                  }
                }
              }

              // Send message
              if (banner != null)
                sendMessage(outObj);
              else
                log.error("Banner is null for abonent " + abonent);

              // Calculate inaccuracy
              inaccurancy = (System.currentTimeMillis() - iterStartTime) - expectedDelay;
              totalInaccurancy -= inaccurancy;
            }

          } catch (DataSourceException e) {
            log.error("Can't get abonents list", e);
          }
        }

        totalAbonentsCount = getTotalAbonentsCount();
      }
    }

    protected void stopCurrentWork() {
      synchronized(startDayWaiter) {
        startDayWaiter.notifyAll();
      }
      synchronized(waitAbonentsObject) {
        waitAbonentsObject.notifyAll();
      }
      synchronized(delayObject) {
        delayObject.notifyAll();
      }
    }

    private void decreaseMessagesCount(String abonent) throws DataSourceException {
      ds.decreaseMessagesCount(abonent);
    }

    private int getTotalMessagesCount()  {
      try {
        return ds.getTotalMessagesCount(subscriptionCount);
      } catch (DataSourceException e) {
        log.error(e,e);
        return 0;
      }
    }

    private int getTotalAbonentsCount() {
      try {
        return ds.getTotalAbonentsCount(subscriptionCount);
      } catch (DataSourceException e) {
        log.error(e,e);
        return 0;
      }
    }

    private int waitAbonents(long untilTime) {
      System.out.println("wait abonents");
      int totalAbonentsCount = getTotalAbonentsCount();
      while(isStarted() && totalAbonentsCount == 0 && System.currentTimeMillis() < untilTime) {
        synchronized(waitAbonentsObject) {
          try {
            waitAbonentsObject.wait(10000);
          } catch (InterruptedException e) {
          }
        }

        totalAbonentsCount = getTotalAbonentsCount();
      }
      return totalAbonentsCount;
    }

    private void sendMessage(SMPPTransportObject outObj) {
      outQueue.addOutgoingObject(outObj);
    }

    private String getBannerForAbonent(String abonent) {
      try {
        return advertisingClient.getBanner(advertisingClientName, abonent);
      } catch (Throwable e) {
        log.error("Can't get banner for abonent", e);
        return null;
      }
    }
  }
}
