package ru.novosoft.smsc.infosme.backend.siebel;

import ru.sibinco.util.conpool.ConnectionPool;

import java.sql.PreparedStatement;
import java.sql.Connection;
import java.sql.Timestamp;
import java.util.Properties;
import java.util.Random;
import java.util.Map;
import java.util.HashMap;
import java.io.InputStream;
import java.io.FileInputStream;

/**
 * author: alkhal
 */
public class CreateDB {

  private final static Random random = new Random();

  private static int wavesize = 1000;
  private static int nwaves = 1;
  private static int firstwaveid = 0;
  private static int firstmsgid = -1;

  public static void main(String[] args) throws Exception{
    int argSize = args.length;
    Map arguments = new HashMap();
    if(argSize%2 == 0) {
      help();
      return;
    }

    if(argSize>1) {
      for(int i=1; i<=argSize/2+1;i+=2) {
        arguments.put(args[i],args[i+1]);
      }
    }
    Properties props = new Properties();
    if(arguments.containsKey("-f")) {
      InputStream is = null;
      try{
        is = new FileInputStream((String)arguments.get("-f"));
        props.load(is);
      }finally{
        if(is != null) {
          try{
            is.close();
          }catch(Throwable e){}
        }
      }
    } else {
      props.setProperty("jdbc.source", "jdbc:mysql://mangal/summer_edu");
      props.setProperty("jdbc.driver", "com.mysql.jdbc.Driver");
      props.setProperty("jdbc.user", "student");
      props.setProperty("jdbc.pass", "student");
    }
    if(arguments.containsKey("-s")) {
      wavesize = Integer.parseInt((String)arguments.get("-s"));
    }
    if(arguments.containsKey("-w")) {
      nwaves = Integer.parseInt((String)arguments.get("-w"));
    }
    if(arguments.containsKey("-n")) {
      firstwaveid = Integer.parseInt((String)arguments.get("-n"));
    }
    firstmsgid = wavesize * firstwaveid;
    if(arguments.containsKey("-m")) {
      firstmsgid = Integer.parseInt((String)arguments.get("-m"));
    }
    if(args[0].equals("create")) {
      createAll(props);
    }else if(args[0].equals("remove")) {
      removeAll(props);
    } else {
      System.out.println("Unknown method: "+args[0]);
      help();
    }
  }

  public static void help() {
    System.out.println("Usage: CreateDB (create|remove) [options]");
    System.out.println("Options:");
    System.out.println("-f config file");
    System.out.println("-w the number of waves (default: 1)");
    System.out.println("-s the number of messages per wave (default: 1000)");
    System.out.println("-n the waveid of the first wave (default: 0)");
    System.out.println("-m the msgid of the first msg (default: waveid*wavesize)");
  }

  public static void createAll(Properties jdbcProp) {
    try{

      ConnectionPool pool = new ConnectionPool(jdbcProp);

      Connection connection = null;
      PreparedStatement prepStatement = null;

      try{
        connection = pool.getConnection();
        connection.setAutoCommit(false);

        int msgid = firstmsgid;
        try {
          prepStatement = connection.prepareStatement("insert into SMS_MAIL (ROW_ID, WAVE_INT_ID, " +
              "CLC_INT_ID, CREATED, LAST_UPD, MSISDN, MESSAGE, MESSAGE_STATE, SMSC_STAT_CODE, SMSC_STAT_VAL) values (?,?,?,?,?,?,?,?,?,?)");
          int count = 0;

          System.out.println("Creation of SMS_MAIL table...");
          for( int wid = firstwaveid; wid < firstwaveid+nwaves; ++wid ) {
            for(int i=0; i<wavesize; i++) {
            prepStatement.setString(1,Integer.toString(msgid));
            prepStatement.setString(2,Integer.toString(wid));
            prepStatement.setString(3,Integer.toString(msgid));
            prepStatement.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
            prepStatement.setTimestamp(5, new Timestamp(System.currentTimeMillis()));
            prepStatement.setString(6, "+" + Long.toString(Long.parseLong("79130000000") + i));
            prepStatement.setString(7, "TestTest");
//            if(random.nextInt(2)==0) {
//              prepStatement.setString(8, SiebelMessage.State.DELIVERED.toString());
//            }else {
              prepStatement.setNull(8, java.sql.Types.VARCHAR);
//            }
//            if(random.nextInt(2)==0) {
//              prepStatement.setString(9, "1");
//            }else {
              prepStatement.setNull(9, java.sql.Types.VARCHAR);
//            }
//            if(random.nextInt(2)==0) {
//              prepStatement.setString(10, "ok");
//            }else {
              prepStatement.setNull(10, java.sql.Types.VARCHAR);
//            }
            prepStatement.addBatch();
            ++msgid;
            count++;
            if(count == 1000) {
              count = 0;
              prepStatement.executeBatch();
            }
            }
          }
          if(count != 0) {
            prepStatement.executeBatch();
          }
          connection.commit();
        }  finally {
          if(prepStatement != null) {
            prepStatement.close();
          }
        }
        System.out.println("Completed: " + (msgid-firstmsgid) + " messages");
        System.out.println();

        try {
          prepStatement = connection.prepareStatement("insert into SMS_MAIL_PARAMS (ROW_ID, WAVE_INT_ID, CAMPAIGN_INT_ID, " +
              "CREATED, LAST_UPD, PRIORITY, FLASH_FLG, SAVE_FLG, BEEP_FLG, EXP_PERIOD, CTRL_STATUS) values (?,?,?,?,?,?,?,?,?,?,?)");

          int count = 0;
          System.out.println("Creation of SMS_MAIL_PARAMS table...");
          for(int wid=firstwaveid;wid<firstwaveid+nwaves;wid++) {
            prepStatement.setString(1,Integer.toString(wid));
            prepStatement.setString(2,Integer.toString(wid));
            prepStatement.setString(3,Integer.toString(wid));
            prepStatement.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
            prepStatement.setTimestamp(5, new Timestamp(System.currentTimeMillis()));
            prepStatement.setString(6, Integer.toString(wid+1));
            prepStatement.setString(7,"Y");
            prepStatement.setString(8,"Y");
            prepStatement.setString(9,"N");
            prepStatement.setString(10, Integer.toString(wid+1));
            prepStatement.setString(11, SiebelTask.Status.ENQUEUED.toString());

            prepStatement.addBatch();
            count++;
            if(count == 1000) {
              count = 0;
              prepStatement.executeBatch();
            }
          }
          if(count != 0) {
            prepStatement.executeBatch();
          }
          connection.commit();

        } finally {
          if(prepStatement != null) {
            prepStatement.close();
          }
        }
        System.out.println("Completed: " + nwaves + " waves");

        System.out.println("Finished");
      }catch (Throwable e) {
        e.printStackTrace();
      }finally {
        connection.close();
      }

    }catch (Throwable e) {
      e.printStackTrace();
    }

  }


  public static void removeAll(Properties jdbcProp) {
    try{

      ConnectionPool pool = new ConnectionPool(jdbcProp);

      Connection connection = null;
      PreparedStatement prepStatement = null;

      try {
        connection = pool.getConnection();

        try{
          prepStatement = connection.prepareStatement("DELETE FROM SMS_MAIL");
          prepStatement.executeUpdate();
        }  finally {
          if(prepStatement != null) {
            prepStatement.close();
          }
        }

        try {
          prepStatement = connection.prepareStatement("DELETE FROM SMS_MAIL_PARAMS");
          prepStatement.executeUpdate();
        }  finally {
          if(prepStatement != null) {
            prepStatement.close();
          }
        }
        System.out.println("Finished");
      }catch (Throwable e) {
        e.printStackTrace();
      }finally {
        try{
          if(connection != null) {
            connection.close();
          }
        }catch (Throwable e){}
      }
    }catch (Throwable e){
      e.printStackTrace();
    }
  }
}

