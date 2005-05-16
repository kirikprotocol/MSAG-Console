package ru.novosoft.smsc.admin.smsexport;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.smsview.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.util.conpool.NSConnectionPool;

import java.io.*;
import java.util.*;
import java.util.Date;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Connection;
import java.sql.DriverManager;
import org.apache.log4j.Category;


import oracle.jdbc.driver.OracleCallableStatement;
import oracle.jdbc.OracleConnection;
import oracle.sql.*;

import javax.sql.DataSource;


/**
 * Created by IntelliJ IDEA.
 * User: andrey
 * Date: 10.02.2005
 * Time: 18:24:47
 * To change this template use File | Settings | File Templates.
 */
public class SmsExport extends SmsSource //implements ORAData
{
  private final static String INSERT_OP_SQL = "INSERT INTO ";
  private final static String UPDATE_OP_SQL = "UPDATE ";
  private final static String DELETE_OP_SQL = "TRUNCATE TABLE ";
  private final static String WHERE_SMS_ID_SQL = " WHERE ID=?";
  private final static String CALL_multinsert_sms_SQL = "{ call multinsert_sms(?,?) }"; //1-integer , 2-msgs in arraylist
  private final static String CREATE_PROC_INSERT_RECORD=" create or replace procedure insert_rec( msg in sms ) is begin ";

  private final static String INSERT_FIELDS=
          " (id,st,submit_time,valid_time,attempts,last_result, "+
	"	last_try_time,next_try_time,oa,da,dda,mr,svc_type,dr,br,src_msc,     "+
	"	src_imsi,src_sme_n,dst_msc,dst_imsi,dst_sme_n,route_id,svc_id,prty,  "+
  "              src_sme_id,dst_sme_id,msg_ref,seq_num,body_len) ";
private final static String INSERT_VALUES =
        "	values (msg.id,msg.st,msg.submit_time,msg.valid_time,msg.attempts, " +
"		msg.last_result,msg.last_try_time,msg.next_try_time,msg.oa,msg.da,  "+
"		msg.dda,msg.mr,msg.svc_type,msg.dr,msg.br,msg.src_msc,   "+
"		msg.src_imsi,msg.src_sme_n,msg.dst_msc,msg.dst_imsi,msg.dst_sme_n, "+
"		msg.route_id,msg.svc_id,msg.prty,msg.src_sme_id,msg.dst_sme_id, "+
               " msg.msg_ref,msg.seq_num,msg.body_len); "+
"end; ";

  Category logger = Category.getInstance(SmsExport.class);
  private String smsstorePath;
  private static final String SECTION_NAME_LocalStore = "MessageStore.LocalStore";
  private static final String PARAM_NAME_filename = "filename";
  private static final String PARAM_NAME_SMSSTORE = "MessageStore.LocalStore.filename";
  private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));

  private ExportSettings defaultExportSettings = null;

  private static Object instanceLock = new Object();
  private static SmsExport instance = null;

  public static SmsExport getInstance(Config webConfig ,Config smscConfig) throws AdminException
  {
    synchronized (instanceLock) {
      if (instance == null) instance = new SmsExport( webConfig , smscConfig);
      return instance;
    }
  }

  protected SmsExport(Config webConfig ,Config smscConfig) throws AdminException
  {
    //Config webConfig = appContext.getConfig(); // webappConfig
     try {
            smsstorePath = smscConfig.getString(PARAM_NAME_SMSSTORE);
            if (smsstorePath == null || smsstorePath.length() <= 0)
                throw new AdminException("store path is empty");
        } catch (Exception e) {
            throw new AdminException("Failed to obtain MessageStore.LocalStore.filename Details: " + e.getMessage());
        }
    try {
      final String section = "opersave_datasource";
      final String source = webConfig.getString(section + ".source");
      final String driver = webConfig.getString(section + ".driver");
      final String user = webConfig.getString(section + ".user");
      final String pass = webConfig.getString(section + ".pass");
      final String prefix = webConfig.getString(section + ".tables_prefix");
      defaultExportSettings = new ExportSettings(source, driver, user, pass, prefix);
    } catch (Exception e) {
      throw new AdminException("Failed to configure default export settings. Details: " + e.getMessage());
    }
     File statPath = new File(smsstorePath);
        if (statPath == null || !statPath.isAbsolute()) {
            File smscConfFile = WebAppFolders.getSmscConfFolder();
            String absolutePath = smscConfFile.getParent();
            statPath = new File(absolutePath, smsstorePath);
            logger.debug("Sms store path: by smsc conf '"+statPath.getAbsolutePath()+"'");
        } else {
            logger.debug("Sms store path: is absolute '"+statPath.getAbsolutePath()+"'");
        }
    smsstorePath = statPath.getAbsolutePath();
   // Smsc smsc = appContext.getSmsc();
  /*  String configPath = smsc.getConfigFolder().getAbsolutePath();
    String dirPrefics = "/"; //Solaris
    int len = configPath.lastIndexOf(dirPrefics) + 1; //Solaris
    if (len < 1) {
      dirPrefics = "\\";//Windows
      len = configPath.lastIndexOf(dirPrefics) + 1;//Windows
    }
    String absolutePath = configPath.substring(0, len);
    //Config smscConfig = smsc.getSmscConfig();

    try {
      smsstorePath = smscConfig.getString(SECTION_NAME_LocalStore + '.' + PARAM_NAME_filename);
    } catch (Config.ParamNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    } catch (Config.WrongParamTypeException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
    if (smsstorePath.indexOf(dirPrefics) != 0)
      smsstorePath = absolutePath + smsstorePath;
  */
    FileInputStream input = null;
    try {
      input = new FileInputStream(smsstorePath);
    } catch (FileNotFoundException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      throw new AdminException(e.getMessage());
    }
  }

  protected void load(File file) throws IOException
  {
    FileReader reader = new FileReader(file);
    reader.read();
  }

  private Object generalLock = new Object();


  public void export(ExportSettings export) throws AdminException
  {
    synchronized (generalLock) {
      _export((export != null) ? export : defaultExportSettings);
    }
  }

  public void export() throws AdminException
  {
    synchronized (generalLock) {
      _export(defaultExportSettings);
    }
  }

  private DataSource createDataSource(ExportSettings export) throws SQLException
  {
    Properties props = new Properties();
    props.setProperty("jdbc.source", export.getSource());
    props.setProperty("jdbc.driver", export.getDriver());
    props.setProperty("jdbc.user", export.getUser());
    props.setProperty("jdbc.pass", export.getPassword());
    return new NSConnectionPool(props);
  }
    private oracle.jdbc.OracleConnection getOracleConnection(ExportSettings export) throws SQLException
  {
    OracleConnection con = null;
    Properties props = new Properties();
    props.setProperty("jdbc.source", export.getSource());
    props.setProperty("jdbc.driver", export.getDriver());
    props.setProperty("jdbc.user", export.getUser());
    props.setProperty("jdbc.pass", export.getPassword());
    return init(con,props, "jdbc");
  }

   public oracle.jdbc.OracleConnection  init(OracleConnection con,Properties config, String cfgKey) throws SQLException
  {

  int m_maxStatements=10;
  boolean m_implicitCachingEnabled=false;
  boolean m_explicitCachingEnabled=false;
  String dbSource;
  String dbDriver;
  String dbUser;
  String dbPassword;

    dbSource = config.getProperty(cfgKey + ".source");
    if (dbSource == null) {
      throw new SQLException(cfgKey + ".source config parameter not specified");
    }
    try {
      // test if driver already registered
      DriverManager.getDriver(dbSource);
    } catch (SQLException ex) {
      dbDriver = config.getProperty(cfgKey + ".driver");
      if (dbDriver == null) {
        throw new SQLException(cfgKey + ".driver config parameter not specified");
      }
      try {
        Class.forName(dbDriver).newInstance();
      } catch (Exception ex1) {
        logger.error("", ex1);
        throw new SQLException("Invalid DB driver class " + dbDriver + " specified in " + cfgKey + ".driver config parameter");
      }
      try {
        DriverManager.getDriver(dbSource);
      } catch (SQLException ex1) {
        logger.error("", ex1);
        throw new SQLException("Specified DB driver class " + dbDriver + " is not valid for source " + dbSource);
      }
    }
    Properties props = new Properties();
    dbUser = config.getProperty(cfgKey + ".user");
    if (dbUser == null) {
      logger.warn(cfgKey + ".user config parameter not specified");
    }
    else {
      props.put("user", dbUser);
    }
    dbPassword = config.getProperty(cfgKey + ".pass");
    if (dbPassword == null) {
      throw new SQLException(cfgKey + ".pass config parameter not specified");
    }
    else {
      props.put("password", dbPassword);
    }
    // init datasource
  //  cpds = new NSConnectionPoolDataSource(dbSource, props);


        con = (oracle.jdbc.OracleConnection)DriverManager.getConnection(dbSource, props);
        if (logger.isDebugEnabled()) logger.debug("Preinit con=" + con);
        (con).setStatementCacheSize(m_maxStatements);
        (con).setImplicitCachingEnabled(m_implicitCachingEnabled);
        (con).setExplicitCachingEnabled(m_explicitCachingEnabled);
      return con;
  }
  public void _export(ExportSettings export) throws AdminException
  {
    final String tablesPrefix = export.getTablesPrefix();
    final String SmsViewTable = tablesPrefix;
    String CLEAR_SQL = DELETE_OP_SQL + SmsViewTable;
    String CREATE_PROC_SQL=CREATE_PROC_INSERT_RECORD+INSERT_OP_SQL+SmsViewTable+INSERT_FIELDS+INSERT_VALUES;
    InputStream input = null;

    HashMap msgsFull = new HashMap(5000);
     long tm=0;
   // DataSource ds = null;
    Connection conn = null;
    PreparedStatement clearStmt = null;
    PreparedStatement createprocStmt = null;
    OracleCallableStatement callinsertStmt = null;
    try {
      input = new FileInputStream(smsstorePath);
     System.out.println("start reading File in: " + new Date());
     tm = System.currentTimeMillis();
      String FileName = Message.readString(input, 9);
      int version = (int) Message.readUInt32(input);
      try {
        SmsFileImport resp = new SmsFileImport();
        byte message[] = new byte[256 * 1024];   int j=0;
        while (true) {
          int msgSize1 = (int) Message.readUInt32(input);
          Functions.readBuffer(input, message, msgSize1);
          int msgSize2 = (int) Message.readUInt32(input);
          if (msgSize1 != msgSize2) throw new AdminException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);
          InputStream bis = new ByteArrayInputStream(message, 0, msgSize1);
          long msgId = Message.readInt64(bis);
          Long lmsgId = new Long(msgId);

          if (resp.receive(bis, msgId)) {
            j++;
            SqlSms sms = resp.getSqlSms();
            if (sms.getStatusInt() == SmsRow.MSG_STATE_ENROUTE) {
              msgsFull.put(lmsgId, sms);
            }
            else {
              msgsFull.remove(lmsgId); //delete sms which to be in time (успели) finalized (финализироваться)
            }
          }
        } //while(true)
      } catch (EOFException e) {
      }
      System.out.println("end reading File in: " + new Date() + " spent: " + (System.currentTimeMillis() - tm) / 1000);
      System.out.println("start clearing old data from Table in: " + new Date());
 //   long tmTable = System.currentTimeMillis();
//      ds = createDataSource(export);
//      conn =ds.getConnection();
      tm = System.currentTimeMillis();
      conn = getOracleConnection(export);

      clearStmt = conn.prepareStatement(CLEAR_SQL);
      clearTable(conn, clearStmt);
      System.out.println("end clearing old data from Table in: " + new Date() + " spent: " + (System.currentTimeMillis() - tm) / 1000);
      tm = System.currentTimeMillis();
      int cnt = 0;   int ArraySize=25000;
      SqlSms msgs[]= new SqlSms[ArraySize] ;
      createprocStmt=conn.prepareStatement(CREATE_PROC_SQL);
      createprocStmt.executeUpdate();
      callinsertStmt=(OracleCallableStatement)conn.prepareCall(CALL_multinsert_sms_SQL);

      Dictionary map = (Dictionary)conn.getTypeMap();
      map.put("sms", Class.forName("ru.novosoft.smsc.admin.smsexport.SqlSms"));
      oracle.sql.ArrayDescriptor ad=ArrayDescriptor.createDescriptor("ARRAYLIST",conn);
      int i=0;
      System.out.println("Inserting "+msgsFull.size()+" records");
      for( Iterator it = msgsFull.values().iterator(); it.hasNext(); ) {
        SqlSms sms1 = (SqlSms) it.next();
        msgs[i]= sms1;
        i++;
        if( ((++cnt) % ArraySize) == 0 ) {
          oracle.sql.ARRAY msgs1= new oracle.sql.ARRAY(ad,conn,msgs);
          callinsertStmt.setInt(1,i);
          callinsertStmt.setARRAY(2,msgs1);
          callinsertStmt.execute();
          System.out.println("Commited: "+cnt);
          i=0;
        }
      }
      if (i>0) {
        oracle.sql.ARRAY msgs1= new oracle.sql.ARRAY(ad,conn,msgs);
        callinsertStmt.setInt(1,i);
        callinsertStmt.setARRAY(2,msgs1);
        callinsertStmt.execute();
        System.out.println("Commited: "+cnt);
      }
    } catch (SQLException e) {
      logger.error("SQL err", e);
      try {
        if( conn != null ) conn.rollback();
      } catch (SQLException e1) {
        logger.warn("rollback failed", e);
      }
      throw new AdminException(e.getMessage());
    } catch (Exception e) {
      try {
        if( conn != null ) conn.rollback();
      } catch (SQLException e1) {
        logger.warn("rollback failed", e);
      }
      logger.error("Unexpected exception occured exporting operative store file", e);
    } finally {
      if (input != null) {
        try {
          input.close();
        } catch (IOException e) {
          logger.warn("can't close file");
        }
      }
      System.out.println("end export in: " + new Date() + " spent: " + (System.currentTimeMillis() - tm) / 1000);
      closeStatement(clearStmt);
    //  closeStatement(insertStmt);
    //  closeStatement(updateStmt);
    //  closeStatement(deleteStmt);
      //closeStatement(callprepareStmt);
      closeStatement(callinsertStmt);
      if (conn != null) {
        try {
          conn.close();
        } catch (SQLException e) {
          logger.warn("Couldn't close connection");
        }
      }
      //  set.addAll(msgs.values());
    }
    // set.setSmesRows(msgs.size());
    //  return set;
  }

  protected void closeStatement(PreparedStatement stmt)
  {
    if (stmt != null) {
      try {
        stmt.close();
      } catch (SQLException e) {
        logger.warn("Couldn't close statement");
      }
    }
  }

  public void clearTable(Connection con, PreparedStatement stmt) throws SQLException
  {
    stmt.executeUpdate();
    con.commit();
  }
/*
  public void addSms(oracle.jdbc.OracleConnection con, PreparedStatement stmt, SmsRow sms) throws SQLException
  {
      bindInput(stmt, sms);
      stmt.executeUpdate();
  }

  public void putSms(oracle.jdbc.OracleConnection con, PreparedStatement stmt, SmsRow sms) throws SQLException
  {
      bindInput(stmt, sms);
      stmt.executeUpdate();
  }

  public void deleteSms(oracle.jdbc.OracleConnection con, PreparedStatement stmt, SmsRow sms) throws SQLException
  {
      stmt.setLong(1, sms.getId());
      stmt.executeUpdate();
  }

  private void bindInput(PreparedStatement stmt, SmsRow sms)
          throws SQLException
  {
    int pos = 1;

    stmt.setInt(pos++, sms.getStatusInt());

    stmt.setTimestamp(pos++, new java.sql.Timestamp(sms.getSubmitTime().getTime()), calendar);
    stmt.setTimestamp(pos++, new java.sql.Timestamp(sms.getValidTime().getTime()), calendar);
    stmt.setTimestamp(pos++, new java.sql.Timestamp(sms.getLastTryTime().getTime()), calendar);
    stmt.setTimestamp(pos++, new java.sql.Timestamp(sms.getNextTryTime().getTime()), calendar);
    stmt.setInt(pos++, sms.getAttempts());
    stmt.setInt(pos++, sms.getLastResult());
    stmt.setString(pos++, sms.getOriginatingAddress());
    stmt.setString(pos++, sms.getDestinationAddress());
    stmt.setString(pos++, sms.getDealiasedDestinationAddress());
    stmt.setInt(pos++, sms.getMessageReference());
    stmt.setString(pos++, sms.getServiceType());
    stmt.setShort(pos++, sms.getDeliveryReport());
    stmt.setShort(pos++, sms.getBillingRecord());

    SmsDescriptor originDescr = sms.getOriginatingDescriptor();
    SmsDescriptor destDescr = sms.getDestinationDescriptor();

    stmt.setString(pos++, originDescr.getMsc());
    stmt.setString(pos++, originDescr.getImsi());
    stmt.setInt(pos++, originDescr.getSme());
    stmt.setString(pos++, destDescr.getMsc());
    stmt.setString(pos++, destDescr.getImsi());
    stmt.setInt(pos++, destDescr.getSme());

    stmt.setString(pos++, sms.getRouteId());
    stmt.setInt(pos++, sms.getServiceId());
    stmt.setInt(pos++, sms.getPriority());
    stmt.setString(pos++, sms.getSrcSmeId());
    stmt.setString(pos++, sms.getDstSmeId());
    //stmt.setInt(pos++, 0); //TXT_LENGTH  deprecated
    stmt.setInt(pos++, sms.getConcatMsgRef()); //concat message reference
    stmt.setInt(pos++, sms.getConcatSeqNum()); //sequense number
    stmt.setInt(pos++, sms.getBodyLen()); //Body_Len
    //stmt.set
    //stmt.setString(pos++,"n"); //field Arc
    stmt.setLong(pos, sms.getId());
  }
 */
  public String getSmsstorePath()
  {
    return smsstorePath;
  }

}
