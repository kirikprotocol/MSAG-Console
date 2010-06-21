package ru.novosoft.smsc.admin.smsexport;

import oracle.jdbc.driver.OracleCallableStatement;
import oracle.sql.ArrayDescriptor;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.smsstat.ExportSettings;
import ru.novosoft.smsc.admin.smsview.SmsRow;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.sql.*;
import java.util.*;
import java.util.Date;

public class SmsOperativeExport extends SmsExport {
    protected static final String DS_OPER_SECTION = "opersave_datasource";
    protected static final String SECTION_NAME_LocalStore = "MessageStore.LocalStore";
    protected static final String PARAM_NAME_filename = "filename";
    protected static final String PARAM_NAME_SMSSTORE = "MessageStore.LocalStore.filename";

    private String smsstorePath;

    protected static Object instanceLock = new Object();
    protected static SmsExport instance = null;

    public static SmsExport getInstance(Config webConfig, Config smscConfig) throws AdminException {
        DS_SECTION = DS_OPER_SECTION;
        synchronized (instanceLock) {
            if (instance == null) instance = new SmsOperativeExport(webConfig, smscConfig);
            return instance;
        }
    }

    protected SmsOperativeExport(Config webConfig, Config smscConfig) throws AdminException {
        super(webConfig);
        try {
            smsstorePath = smscConfig.getString(PARAM_NAME_SMSSTORE);
            if (smsstorePath == null || smsstorePath.length() <= 0)
                throw new AdminException("store path is empty");
        } catch (Exception e) {
            throw new AdminException("Failed to obtain MessageStore.LocalStore.filename Details: " + e.getMessage());
        }
        File statPath = new File(smsstorePath);
        if (statPath == null || !statPath.isAbsolute()) {
            File smscConfFile = WebAppFolders.getSmscConfFolder();
            String absolutePath = smscConfFile.getParent();
            statPath = new File(absolutePath, smsstorePath);
            logger.debug("Sms store path: by smsc conf '" + statPath.getAbsolutePath() + "'");
        } else {
            logger.debug("Sms store path: is absolute '" + statPath.getAbsolutePath() + "'");
        }
        smsstorePath = statPath.getAbsolutePath();

        try {
            new FileInputStream(smsstorePath);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            throw new AdminException(e.getMessage());
        }
    }

    public oracle.jdbc.OracleConnection getOracleConnection(ExportSettings export) throws SQLException {

        int m_maxStatements = 10;
        boolean m_implicitCachingEnabled = false;
        boolean m_explicitCachingEnabled = false;
        String dbSource;
        String dbDriver;
        String dbUser;
        String dbPassword;

        dbSource = export.getSource();
        if (dbSource == null) {
            throw new SQLException("jdbc.source config parameter not specified");
        }
        try {
            DriverManager.getDriver(dbSource);
        } catch (SQLException ex) {
            dbDriver = export.getDriver();
            if (dbDriver == null) {
                throw new SQLException("jdbc.driver config parameter not specified");
            }
            try {
                Class.forName(dbDriver).newInstance();
            } catch (Exception ex1) {
                logger.error("", ex1);
                throw new SQLException("Invalid DB driver class " + dbDriver + " specified in jdbc.driver config parameter");
            }
            try {
                DriverManager.getDriver(dbSource);
            } catch (SQLException ex1) {
                logger.error("", ex1);
                throw new SQLException("Specified DB driver class " + dbDriver + " is not valid for source " + dbSource);
            }
        }
        Properties props = new Properties();
        dbUser = export.getUser();
        if (dbUser == null) {
            logger.warn("jdbc.user config parameter not specified");
        } else {
            props.put("user", dbUser);
        }
        dbPassword = export.getPassword();
        if (dbPassword == null) {
            throw new SQLException("jdbc.pass config parameter not specified");
        } else {
            props.put("password", dbPassword);
        }

        oracle.jdbc.OracleConnection con = (oracle.jdbc.OracleConnection) DriverManager.getConnection(dbSource, props);
        logger.debug("Preinit con=" + con);
        (con).setStatementCacheSize(m_maxStatements);
        (con).setImplicitCachingEnabled(m_implicitCachingEnabled);
        (con).setExplicitCachingEnabled(m_explicitCachingEnabled);
        return con;
    }

    protected void _export(ExportSettings export) throws AdminException {
        final String tablesPrefix = export.getTablesPrefix();
        String CREATE_PROC_SQL = CREATE_PROC_INSERT_RECORD + INSERT_OP_SQL + tablesPrefix + INSERT_FIELDS + INSERT_VALUES;
        InputStream input = null;

        HashMap msgsFull = new HashMap(5000);
        long tm = 0;
        Connection conn = null;
        PreparedStatement createprocStmt = null;
        OracleCallableStatement callinsertStmt = null;
        boolean haveArc = false;
        try {
            input = new BufferedInputStream(new FileInputStream(smsstorePath));
            System.out.println("start reading File in: " + new Date());
            tm = System.currentTimeMillis();
            String FileName = Message.readString(input, 9);
            int version = (int) Message.readUInt32(input);
            if (version > 0x010000) haveArc = true;
            try {
                SmsFileImport resp = new SmsFileImport();
                byte message[] = new byte[256 * 1024];
                int j = 0;
                while (true) {
                    int msgSize1 = (int) Message.readUInt32(input);
                    Functions.readBuffer(input, message, msgSize1);
                    int msgSize2 = (int) Message.readUInt32(input);
                    if (msgSize1 != msgSize2) throw new AdminException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);
                    InputStream bis = new ByteArrayInputStream(message, 0, msgSize1);
                    long msgId = Message.readInt64(bis);
                    Long lmsgId = new Long(msgId);

                    Message.readUInt32(bis); // Skip seq
                    Message.readString(bis,1); // Skip final

                    int status = Message.readUInt8(bis);

                    j++;
                    if (status == SmsRow.MSG_STATE_ENROUTE) {
                      Sms sms = new Sms(msgId, status, bis);
                      msgsFull.put(lmsgId, sms);
                    } else
                      msgsFull.remove(lmsgId);


                } //while(true)
            } catch (EOFException e) {
            }
            System.out.println("end reading File in: " + new Date() + " spent: " + (System.currentTimeMillis() - tm) / 1000);
            System.out.println("start clearing old data from Table in: " + new Date());
            tm = System.currentTimeMillis();
            conn = getOracleConnection(export);

            clearTable(conn, tablesPrefix, null);
            System.out.println("end clearing old data from Table in: " + new Date() + " spent: " + (System.currentTimeMillis() - tm) / 1000);
            tm = System.currentTimeMillis();
            int cnt = 0;
            int ArraySize = 25000;
            SqlSms msgs[] = new SqlSms[ArraySize];
            createprocStmt = conn.prepareStatement(CREATE_PROC_SQL);
            createprocStmt.executeUpdate();
            callinsertStmt = (OracleCallableStatement) conn.prepareCall(CALL_multinsert_sms_SQL);

            Dictionary map = (Dictionary) conn.getTypeMap();
            map.put("sms", Class.forName("ru.novosoft.smsc.admin.smsexport.SqlSms"));
            oracle.sql.ArrayDescriptor ad = ArrayDescriptor.createDescriptor("ARRAYLIST", conn);
            int i = 0;
            System.out.println("Inserting " + msgsFull.size() + " records");
            for (Iterator it = msgsFull.values().iterator(); it.hasNext();) {
              try {
                msgs[i] = ((Sms) it.next()).toSqlSms(haveArc);
              } catch (IOException e) {
                continue;
              }
                i++;
                cnt++;
                if (i == ArraySize) {
                oracle.sql.ARRAY msgs1 = new oracle.sql.ARRAY(ad, conn, msgs);
                    callinsertStmt.setInt(1, i);
                    callinsertStmt.setARRAY(2, msgs1);
                    callinsertStmt.execute();
                    System.out.println("Commited: " + cnt);
                    i = 0;
                }
            }
            if (i > 0) {
                oracle.sql.ARRAY msgs1 = new oracle.sql.ARRAY(ad, conn, msgs);
                callinsertStmt.setInt(1, i);
                callinsertStmt.setARRAY(2, msgs1);
                callinsertStmt.execute();
                System.out.println("Commited: " + cnt);
            }
        } catch (SQLException e) {
            logger.error("SQL err", e);
            try {
                if (conn != null) conn.rollback();
            } catch (SQLException e1) {
                logger.warn("rollback failed", e);
            }
            throw new AdminException(e.getMessage());
        } catch (Exception e) {
            try {
                if (conn != null) conn.rollback();
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
            closeStatement(callinsertStmt);
            if (conn != null) {
                try {
                    conn.close();
                } catch (SQLException e) {
                    logger.warn("Couldn't close connection");
                }
            }
        }
    }

  private static class Sms {
    private final long msgId;
    private final InputStream is;
    private final int status;

    private Sms(long msgId, int status, InputStream bytes) throws IOException {
      this.msgId = msgId;
      this.is = bytes;
      this.status = status;
    }

    public SqlSms toSqlSms(boolean haveArc) throws IOException {
      java.sql.Date submitTime = new java.sql.Date(Message.readUInt32(is)*1000);

      long validTime = Message.readUInt32(is);
      long lastTryTime = Message.readUInt32(is);
      long nextTryTime = Message.readUInt32(is);
      int attempts = (int)Message.readUInt32(is);

      int lastResult = (int)Message.readUInt32(is);

      int strLen = Message.readUInt8(is);
      Mask origMask = new Mask((byte)Message.readUInt8(is), (byte)Message.readUInt8(is), Message.readString(is,strLen));

      strLen = Message.readUInt8(is);
      Mask destMask = new Mask((byte)Message.readUInt8(is), (byte)Message.readUInt8(is), Message.readString(is,strLen));

      strLen = Message.readUInt8(is);
      Mask dDestMask = new Mask((byte)Message.readUInt8(is), (byte)Message.readUInt8(is), Message.readString(is,strLen));

      int mr = Message.readUInt16(is);
      String sctype = Message.readString8(is);
      short dreport = (short) Message.readUInt8(is);
      short billr = (short) Message.readUInt8(is);
      String odMsc = Message.readString8(is);
      String odImsi = Message.readString8(is);
      int odSme = (int) Message.readUInt32(is);
      String ddMsc = Message.readString8(is);
      String ddImsi = Message.readString8(is);
      int ddSme = (int) Message.readUInt32(is);

      String routeId = Message.readString8(is);

      int serviceId = (int) Message.readUInt32(is);
      int priority = (int) Message.readUInt32(is);

      String srcSmeId = Message.readString8(is);
      String dstSmeId = Message.readString8(is);


      short concatMsgRef=(short)Message.readUInt16(is);

      short concatSeqNum=(short)Message.readUInt8(is);

      byte arc=-1;
      if (haveArc)  {
        arc=((byte)Message.readUInt8(is));
      }
      int bodyLen = (int) Message.readUInt32(is);

      return new SqlSms("SMS",msgId,status,submitTime,new java.sql.Date(validTime*1000),attempts,
                lastResult,new java.sql.Date(lastTryTime*1000),new java.sql.Date(nextTryTime*1000),origMask.getMask(),destMask.getMask(),
                dDestMask.getMask(),mr,sctype,dreport,
                billr,odMsc,odImsi,odSme,ddMsc,
                ddImsi,ddSme,routeId,serviceId,priority,
               srcSmeId,dstSmeId,concatMsgRef,concatSeqNum,arc,bodyLen);
    }
  }
}