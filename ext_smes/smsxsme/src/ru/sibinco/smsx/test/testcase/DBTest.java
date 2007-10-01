package ru.sibinco.smsx.test.testcase;

import snaq.db.ConnectionPool;

import java.sql.*;

/**
 * User: artem
 * Date: Aug 29, 2007
 */

public class DBTest {
  public static void main(String[] args) {


    try {
//      Class.forName("com.mysql.jdbc.Driver");
      DriverManager.registerDriver((Driver)Class.forName("com.mysql.jdbc.Driver").newInstance());
    } catch (Exception e) {
      e.printStackTrace();
      return;
    }

    final String url = "jdbc:mysql://localhost/smsx?useUnicode=true&characterEncoding=UTF-8&autoReconnect=true";

    final ConnectionPool pool = new ConnectionPool("test", 10, 30, 5000, url, "root", "laefeeza");

    final String sql = "INSERT INTO sponsored_distribution_info(abonent,tm, today_cnt, cur_cnt, cnt) values(?,?,?,?,?)";
//    final String sql = "update sponsored_distribution_info set cur_cnt=cur_cnt-1 where abonent=?";

    final int count = 10000;

    final Timestamp ts = new Timestamp(System.currentTimeMillis());

    Connection conn = null;
    PreparedStatement ps =  null;

    long start = 0;

      try {
        pool.init(10);
        pool.setCaching(false, true, false);

        start = System.currentTimeMillis();
        int cnt;

        for (int i=100000; i < 100000 + count; i++) {
          try {
            conn = pool.getConnection();

            ps = conn.prepareStatement(sql);

            ps.setString(1,"+79139" + i);
            ps.setTimestamp(2, ts);
            ps.setInt(3, 20);
            ps.setInt(4, 20);
            ps.setInt(5, 20);

            cnt = ps.executeUpdate();

          } catch (SQLException e) {
            System.out.println(e.getMessage());
            e.printStackTrace();

          } finally {
            try {
              if (ps != null) {
                ps.close();
                ps = null;
              }
            } catch (SQLException e) {
              e.printStackTrace();
            }
            try {
              if (conn != null) {
                conn.close();
                conn = null;
              }
            } catch (SQLException e) {
              e.printStackTrace();
            }

          }
        }

      } finally {

        pool.release();
      }


    System.out.println("Total tspeed = " + count * 1000 / (System.currentTimeMillis() - start) + " p/s");

  }
}
