package ru.sibinco.smsx.tools;

import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.sql.*;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import snaq.db.ConnectionPool;

/**
 * User: artem
 * Date: 23.07.2007
 */

public class SponsoredExport {

  private static final SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd");

//  private static final String sql = "SELECT DISTINCT h1.abonent, h2.start_cnt FROM sponsored_subscription_history h1\n" +
//      "    INNER JOIN (\n" +
//      "      SELECT h21.abonent, h21.cnt start_cnt FROM sponsored_subscription_history h21\n" +
//      "          INNER JOIN (\n" +
//      "            SELECT abonent, max(start_date) sd FROM sponsored_subscription_history WHERE start_date <= ? GROUP BY abonent\n" +
//      "          ) h22 on (h21.abonent = h22.abonent AND h21.start_date=h22.sd)\n" +
//      "    ) h2 ON (h1.abonent=h2.abonent)\n" +
//      "\n" +
//      "    LEFT JOIN (\n" +
//      "      SELECT abonent, count(DISTINCT cnt) changes FROM sponsored_subscription_history WHERE start_date >= ? AND start_date <= ? GROUP BY abonent\n" +
//      "    ) h3 ON (h2.abonent=h3.abonent)\n" +
//      "\n" +
//      "    LEFT JOIN (\n" +
//      "     SELECT abonent, cnt FROM sponsored_subscription_history WHERE start_date >= ? AND start_date <= ? GROUP BY abonent\n" +
//      "   ) h4 ON (h1.abonent=h4.abonent AND h2.start_cnt=h4.cnt) \n" +
//      "\n" +
//      "WHERE ((h3.changes is null) OR (h3.changes=1 AND not (h4.cnt is null))) LIMIT ?,? ";
  private static final String sql = "select address, sum(cnt) from sponsored_delivery_stats where date > ? and date < ? GROUP BY address LIMIT ?,?";

  private static final long DAY_COUNT = 3600 * 1000 * 24;
  private static final int LIMIT=10000;

  private static ConnectionPool pool;

  private static void unload(Date startDate, Date endDate, String fileDir, float cost) {
    Timestamp start = new Timestamp(getDayStartTime(startDate.getTime()).getTime());
    Timestamp end = new Timestamp(getDayStartTime(endDate.getTime()).getTime());


    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    FileWriter writer = null;
    FileWriter log = null;

    try {
      writer = new FileWriter(new File(fileDir, "extra" + df.format(end) + ".csv"));

//      log = new FileWriter(new File(fileDir, "extra" + df.format(end) + ".log"));
//
//      log.write("Unload started at " + new Date() + "\n");
//      log.write("Start date = " + start + "\n");
//      log.write("End date = " + end + "\n");
//      log.write("================================================================================================\n");

      conn = pool.getConnection();

      int count;
      int startPos = 0;
      do {
        ps = conn.prepareStatement(sql);
        ps.setTimestamp(1, start);
        ps.setTimestamp(2, end);
        ps.setInt(3, startPos);
        ps.setInt(4, LIMIT);

        rs = ps.executeQuery();

        String abonent;
        int cnt;
        float totalCost;
        count = 0;
        while(rs.next()) {
          count ++;
          abonent = rs.getString(1);
          if (abonent == null)
            continue;
          abonent = abonent.trim();
          if (abonent.startsWith("+"))
            abonent = abonent.substring(1);

          cnt = rs.getInt(2);
          totalCost = cost * 100 * cnt / 100;

          if (cnt > 0) {
//            log.write(abonent + "|" + totalCost + "\n");
            writer.write(abonent + "|" + totalCost + "\n");
//            log.write("|" + totalCost + "|||" + "bonus SMS Extra " + abonent + "\n");
//            writer.write("|" + totalCost + "|||" + "bonus SMS Extra " + abonent + "\n");
          }
        }

        startPos += LIMIT;
        rs.close();
        ps.close();
        System.out.println(count);
      } while (count > 0);

      System.out.println("Messages was unloaded correctly");

    } catch (Throwable e) {
      e.printStackTrace();
      // e.printStackTrace(new PrintWriter(log));
    } finally {
      try {
        if (rs != null)
          rs.close();
      } catch (Exception e) {
      }
      try {
        if (ps != null)
          ps.close();
      } catch (Exception e) {
      }
      try {
        if (conn != null)
          conn.close();
      } catch (Exception e) {
      }
      try {
        if (writer != null)
          writer.close();
      } catch (Exception e) {
      }
//      try {
//        if (log != null)
//          log.close();
//      } catch (Exception e) {
//      }

      pool.release();
    }
  }

  private static Date getDayStartTime(long time) {
    final Calendar calendar = Calendar.getInstance();
    final Calendar dayStart = Calendar.getInstance();
    calendar.setTimeInMillis(time);
    dayStart.set(Calendar.DAY_OF_MONTH, calendar.get(Calendar.DAY_OF_MONTH));
    dayStart.set(Calendar.MONTH, calendar.get(Calendar.MONTH));
    dayStart.set(Calendar.YEAR, calendar.get(Calendar.YEAR));
    dayStart.set(Calendar.HOUR_OF_DAY, 0);
    dayStart.set(Calendar.MINUTE, 0);
    dayStart.set(Calendar.SECOND, 0);
    dayStart.set(Calendar.MILLISECOND, 0);
    return dayStart.getTime();
  }

  public static void insertAbonents() {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      final Timestamp date = new Timestamp(System.currentTimeMillis());
      String address;
      for (int i=0; i < 1000000; i++) {
        address ="+7913" + i;
        ps = conn.prepareStatement("INSERT INTO sponsored_delivery_stats(address, date, cnt) VALUES (?, ?, 10)");

        ps.setString(1, address);
        ps.setTimestamp(2, date);

        ps.executeUpdate();

        ps.close();

        double d = i;

        if (Math.round(i / 10000) == d/10000)
          System.out.println(i);
      }

    } catch (SQLException e) {
      e.printStackTrace();
    } finally {
      try {
        if (ps != null)
          ps.close();
      } catch (Exception e) {
      }
      try {
        if (conn != null)
          conn.close();
      } catch (Exception e) {
      }
    }
  }

  public static void main(String[] args) {

    String configDir;
    String outputDir;
    Date endDate, startDate;
    float cost;

    try {
      configDir = args[0];
      outputDir = args[1];
//      startDate = df.parse(args[2]);
//      endDate = df.parse(args[3]);
      endDate = getDayStartTime(System.currentTimeMillis());
      startDate = new Date(endDate.getTime() - Integer.parseInt(args[2])*DAY_COUNT);

      cost = Float.parseFloat(args[3]);
    } catch (Throwable e) {
      System.out.println("invalid arguments");
      e.printStackTrace();
      return;
    }

    ConnectionPoolFactory.init(configDir);
    pool = ConnectionPoolFactory.createConnectionPool("", 1, 10000);

    pool.init(1);
//    insertAbonents();
    unload(startDate, endDate, outputDir, cost);

  }
}
