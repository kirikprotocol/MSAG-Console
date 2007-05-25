package ru.sibinco.smsx.services.sponsored;

import ru.sibinco.smsx.utils.Service;
import ru.sibinco.smsx.utils.ConnectionPool;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.network.OutgoingObject;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Address;
import ru.aurorisoft.smpp.SMPPAddressException;
import org.apache.log4j.Category;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.*;
import java.io.UnsupportedEncodingException;

import com.lorissoft.advertising.syncclient.AdvertisingClientImpl;
import com.lorissoft.advertising.util.Encode;

/**
 * User: artem
 * Date: 12.03.2007
 */

final class SponsoredEngineNew {
  private static final Category log = Category.getInstance(SponsoredEngineNew.class);

  private static final int ABONENTS_PORTION = 1000;

  private final OutgoingQueue outQueue;

  private final HashMap counters;


  public SponsoredEngineNew(final OutgoingQueue outQueue) {
    this.outQueue = outQueue;

    counters = new HashMap();

    for (int i=0; i < SponsoredServiceNew.Properties.ALLOWED_COUNTS.length; i++) {
      final Messages2SendCounter counter = new Messages2SendCounter();
      final AbonentsGroupWorker worker = new AbonentsGroupWorker(counter, SponsoredServiceNew.Properties.ALLOWED_COUNTS[i]);
      worker.startService();
      worker.waitStarting();
      counters.put(new Integer(SponsoredServiceNew.Properties.ALLOWED_COUNTS[i]), counter);
    }
  }

  public void abonentSubscribed(int messagesCount, long value) {
    ((Messages2SendCounter)counters.get(new Integer(messagesCount))).increase(value);
  }

  public void abonentUnsubscribed(int messagesCount, long value) {
    ((Messages2SendCounter)counters.get(new Integer(messagesCount))).decrease(value);
  }








  private class AbonentsGroupWorker extends Service{

    private final Messages2SendCounter counter;
    private final int subscriptionCount;
    private final AdvertisingClientImpl  advertizingClient;

    public AbonentsGroupWorker(Messages2SendCounter counter, int subscriptionCount) {
      super(log);
      this.counter = counter;
      this.subscriptionCount = subscriptionCount;
      this.advertizingClient = new AdvertisingClientImpl();
      advertizingClient.init(SponsoredServiceNew.Properties.Engine.ADVERTISING_PROPERTIES);
    }

    public synchronized void iterativeWork() {
      // Waiting for day start

      final Calendar dayStart = Calendar.getInstance();
      dayStart.setTime(new Date());

      if (dayStart.get(Calendar.HOUR_OF_DAY) >= SponsoredServiceNew.Properties.Engine.TO_HOUR)
        dayStart.setTimeInMillis(dayStart.getTimeInMillis() + 24*3600000); // Add one day

      dayStart.set(Calendar.HOUR_OF_DAY, SponsoredServiceNew.Properties.Engine.FROM_HOUR);
      dayStart.set(Calendar.MINUTE, 0);
      dayStart.set(Calendar.SECOND, 0);
      dayStart.set(Calendar.MILLISECOND, 0);

      long timeout = dayStart.getTimeInMillis() - System.currentTimeMillis();
      if (timeout > 0) {
        log.debug("Waiting for day start: " + timeout);
        try {
          wait(timeout);
        } catch (InterruptedException e) {
          log.error("Interrupted:", e);
        }
      }

      final Calendar endTime = Calendar.getInstance();
      endTime.setTime(new Date());
      endTime.set(Calendar.HOUR_OF_DAY, SponsoredServiceNew.Properties.Engine.TO_HOUR);
      endTime.set(Calendar.MINUTE, 0);
      endTime.set(Calendar.SECOND, 0);
      endTime.set(Calendar.MILLISECOND, 0);

      int totalCount = waitAbonents(endTime.getTimeInMillis());

      while (totalCount > 0) {

        counter.setCounter(getTotalMessagesCount()); // update messages counter

        for (int j=0; j < totalCount; j+=ABONENTS_PORTION) {

          Connection conn = null;
          PreparedStatement ps = null;
          ResultSet rs = null;

          // Load abonents
          try {
            conn = ConnectionPool.getConnection();
            ps = conn.prepareStatement(SponsoredServiceNew.Properties.Engine.SELECT_ABONENTS_QUERY);
            ps.setInt(1, subscriptionCount);
            ps.setInt(2, j);
            ps.setInt(3, ABONENTS_PORTION);

            rs = ps.executeQuery();

            while (rs.next()) {
              final String abonent = rs.getString(1);
              final String banner = getBannerForAbonent(abonent);
              if (banner != null) {
                sendMessage(banner, abonent);
                decreaseMessagesCount(abonent);
              } else {
                log.error("Banner is null for abonent " + abonent);
              }

              long start = System.currentTimeMillis();
              long alreadyWaited = 0;
              while(true) {
                if (counter.getCounter() == 0)
                  return;
                long waitInterval = (endTime.getTimeInMillis() - System.currentTimeMillis()) / counter.getCounter() - alreadyWaited;
//                System.out.println("waiting " + waitInterval);
                if (waitInterval > 0)
                  counter.waiting(waitInterval);
                else
                  break;
                alreadyWaited = System.currentTimeMillis() - start;
              }
            }

          } catch (SQLException e) {
            log.error("Can't get abonents list", e);
          } finally {
            close(rs, ps, conn);
          }

        }

        totalCount = getTotalAbonentsCount();
      }
    }

    private void decreaseMessagesCount(String abonent) {
      counter.decrease(1);

      Connection conn = null;
      PreparedStatement ps = null;

      try {
        conn = ConnectionPool.getConnection();
        ps = conn.prepareStatement(SponsoredServiceNew.Properties.Engine.DECREASE_MESSAGES_COUNT);
        ps.setString(1, abonent);
        ps.executeUpdate();
      } catch (SQLException e) {
        log.error("Can't process day start", e);
      } finally {
        close(null, ps, conn);
      }
    }

    private int getTotalMessagesCount() {

      Connection conn = null;
      PreparedStatement ps = null;
      ResultSet rs = null;

      try {
        conn = ConnectionPool.getConnection();
        ps = conn.prepareStatement(SponsoredServiceNew.Properties.Engine.SELECT_TOTAL_MESSAGES);
        ps.setInt(1, subscriptionCount);
        rs = ps.executeQuery();
        if (rs.next())
          return rs.getInt(1);
      } catch (SQLException e) {
        log.error("Can't get total messages count", e);
      } finally {
        close(rs, ps, conn);
        rs = null;
        ps = null;
        conn = null;
      }

      return 0;
    }

    private int getTotalAbonentsCount() {

      Connection conn = null;
      PreparedStatement ps = null;
      ResultSet rs = null;

      try {
        conn = ConnectionPool.getConnection();
        ps = conn.prepareStatement(SponsoredServiceNew.Properties.Engine.SELECT_TOTAL_ABONENTS);
        ps.setInt(1, subscriptionCount);
        rs = ps.executeQuery();
        if (rs.next())
          return rs.getInt(1);

      } catch (SQLException e) {
        log.error("Can't get total abonents count", e);
      } finally {
        close(rs, ps, conn);
        rs = null;
        ps = null;
        conn = null;
      }

      return 0;
    }

    private int waitAbonents(long untilTime) {
      int totalAbonentsCount = getTotalAbonentsCount();
      while(totalAbonentsCount == 0 && System.currentTimeMillis() < untilTime) {
        counter.waiting(10000);
        totalAbonentsCount = getTotalAbonentsCount();
      }
      return totalAbonentsCount;
    }

    private void close(final ResultSet rs, final PreparedStatement ps, final Connection conn) {
      try {
        if (rs != null)
          rs.close();
        if (ps != null)
          ps.close();
        if (conn != null) {
          conn.commit();
          conn.close();
        }
      } catch (SQLException e) {
        log.error("Can't close: " , e);
      }
    }

    private void sendMessage(final String msg, final String toAbonent) {
      final Message message = new Message();
      message.setSourceAddress(SponsoredServiceNew.Properties.Engine.SME_ADDRESS);
      message.setDestinationAddress(toAbonent);
      message.setMessageString(msg);
      outQueue.addOutgoingObject(new OutgoingObject(message));
    }

    private String getBannerForAbonent(final String abonentAddress) {
      try {
        final Address address = new Address(abonentAddress);
        final String abonent = "." + address.getTon() + "." + address.getNpi() + "." + address.getAddress();

        final byte[] banner = advertizingClient.getLikelyBanner(abonent.getBytes("ISO8859-1"), abonent.length(), SponsoredServiceNew.Properties.Engine.ADVERTISING_SERVICE_NAME.getBytes("ISO8859-1"), 1, 0, 2, 1, 0);
        if (banner != null)
          return Encode.decodeGSM(banner, true);
      } catch (SMPPAddressException e) {
        log.error("Can't get banner for abonent", e);
      } catch (UnsupportedEncodingException e) {
        log.error("Can't get banner for abonent", e);
      }

      return null;
    }
  }
}
