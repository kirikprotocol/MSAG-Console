package ru.novosoft.smsc.admin.operative_store;

import oracle.jdbc.driver.OracleCallableStatement;
import oracle.sql.ArrayDescriptor;
import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCSms;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.DBExportSettings;
import ru.novosoft.smsc.admin.util.ProgressObserver;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Класс, позволяющий читать оперативные стораджи СМСЦ
 *
 * @author Artem Snopkov
 */
public class OperativeStoreManager {

  private static final Logger logger = Logger.getLogger(OperativeStoreManager.class);

  private final File[] smsStorePaths;
  private final FileSystem fs;

  private final ClusterController cc;

  public OperativeStoreManager(File[] smsStorePaths, FileSystem fs, ClusterController cc) {
    this.smsStorePaths = smsStorePaths;
    this.fs = fs;
    this.cc = cc;
  }

  private static boolean addressConfirm(Address mask, Address address) {
    String maskStr = mask.getNormalizedAddress();
    String addressStr = address.getNormalizedAddress();
    if (addressStr.length() != maskStr.length())
      return false;

    for (int i = 0; i < maskStr.length(); i++) {
      char mc = maskStr.charAt(i);
      char ac = addressStr.charAt(i);
      if (mc != ac && mc != '?')
        return false;
    }
    return true;
  }

  private static File[] listStoreFiles(final FileSystem fs, File baseFile) {
    String fn = baseFile.getName();
    int i = fn.lastIndexOf('.');
    final String filenamePrefix = i > 0 ? fn.substring(0, i) : fn;

    File[] files = fs.listFiles(baseFile.getParentFile(), new FileFilter() {
      public boolean accept(File pathname) {
        return !fs.isDirectory(pathname) && pathname.getName().startsWith(filenamePrefix);
      }
    });

    if (files == null) {
      return null;
    }
    Arrays.sort(files);
    return files;
  }

  private static boolean isAllowed(Message sms, MessageFilter messageFilter) {
    boolean allowed = true;
    if (messageFilter != null) {
      if (messageFilter.getFromDate() != null && sms.getSubmitTime().compareTo(messageFilter.getFromDate()) < 0) {
        allowed = false;
      } else if (messageFilter.getTillDate() != null && sms.getSubmitTime().compareTo(messageFilter.getTillDate()) >= 0) {
        allowed = false;
      } else if (messageFilter.getFromAddress() != null && !addressConfirm(messageFilter.getFromAddress(), sms.getOriginatingAddress())) {
        allowed = false;
      } else if (messageFilter.getToAddress() != null && !addressConfirm(messageFilter.getToAddress(), sms.getDealiasedDestinationAddress())) {
        allowed = false;
      } else if (messageFilter.getAbonentAddress() != null && !(addressConfirm(messageFilter.getAbonentAddress(), sms.getOriginatingAddress()) || addressConfirm(messageFilter.getAbonentAddress(), sms.getDealiasedDestinationAddress()))) {
        allowed = false;
      } else if (messageFilter.getRouteId() != null && !messageFilter.getRouteId().equals(sms.getRouteId())) {
        allowed = false;
      } else if (messageFilter.getSrcSmeId() != null && !messageFilter.getSrcSmeId().equals(sms.getSrcSmeId())) {
        allowed = false;
      } else if (messageFilter.getDstSmeId() != null && !messageFilter.getDstSmeId().equals(sms.getDstSmeId())) {
        allowed = false;
      } else if (messageFilter.getSmeId() != null && !(messageFilter.getSmeId().equals(sms.getSrcSmeId()) || messageFilter.getSmeId().equals(sms.getDstSmeId()))) {
        allowed = false;
      } else if (!messageFilter.additionalFilter(sms)) {
        allowed = false;
      } else if (messageFilter.getLastResult() != null && !messageFilter.getLastResult().equals(sms.getLastResult())) {
        allowed = false;
      }
    }
    return allowed;
  }

  private static void getMessages(File smsStore, FileSystem fs, MessageFilter messageFilter, ProgressObserver observer, Map<Long, Message> msgs, Set<Long> finishedMsgs, long delay, int sleepAfter) throws AdminException {

    int maxSize = messageFilter == null ? Integer.MAX_VALUE : messageFilter.getMaxRowSize();

    long currentPos = 0;

    InputStream input = null;
    try {
      input = new BufferedInputStream(fs.getInputStream(smsStore));

      IOUtils.readString(input, 9);
      IOUtils.readUInt32(input); //read version

      currentPos += 9 + 4;

      try {
        long step = 0;

        //noinspection InfiniteLoopStatement

        boolean outOfSize = false;
        int sleepCounter = 0;
        while (true) {
          if (delay != 0) {
            if (sleepCounter >= sleepAfter) {
              sleepCounter = 0;
              try {
                Thread.sleep(delay);
              } catch (InterruptedException e) {
                return;
              }
            }
            sleepCounter++;
          }
          int msgSize1 = (int) IOUtils.readUInt32(input);
          long msgId = IOUtils.readInt64(input);  // 8 bytes

          boolean outOfSize1 = (msgs.size() == maxSize && !msgs.containsKey(msgId));
          outOfSize = outOfSize || outOfSize1;

          if ((messageFilter != null && messageFilter.getSmsId() != null && msgId != messageFilter.getSmsId()) || finishedMsgs.contains(msgId) || outOfSize1) {
            IOUtils.skip(input, msgSize1 - 8 + 4);
            continue;
          }

          IOUtils.skip(input, 4); // Skip seq (4 bytes)

          int fin = IOUtils.readUInt8(input); // finall (1 byte)
          int status = IOUtils.readUInt8(input); // 1 byte

          if (fin == 0) {
            byte[] message = new byte[msgSize1 - 8 - 5 - 1];
            IOUtils.readFully(input, message, msgSize1 - 8 - 5 - 1);

            LazyMessageImpl sms = new LazyMessageImpl(message, msgId, status);

            boolean allowed = isAllowed(sms, messageFilter);

            if (allowed && !outOfSize1)
              msgs.put(msgId, sms);


          } else {
            msgs.remove(msgId);
            finishedMsgs.add(msgId);
            IOUtils.skip(input, msgSize1 - 8 - 5 - 1);
          }

          int msgSize2 = (int) IOUtils.readUInt32(input);
          if (msgSize1 != msgSize2)
            throw new IOException("Protocol error sz1=" + msgSize1 + " sz2=" + msgSize2);

          currentPos += msgSize1 + 4 + 4;

          if (observer != null) {
            long s = currentPos / 2048000;
            if (s != step) {
              step = s;
              if (currentPos < 0) {
                System.out.println("CATCH IT: currentPosition=" + currentPos);
              }
              observer.update(100 * currentPos / smsStore.length(), 100);
            }
          }
        }


      } catch (EOFException ignored) {
      }

    } catch (IOException e) {
      throw new OperativeStoreException("message_read_error", e);
    } finally {
      if (input != null)
        try {
          input.close();
        } catch (IOException ignored) {
        }
    }

    if (observer != null) {
      observer.update(100, 100);
    }
  }


  static Collection<Message> getMessages(File smsStore, FileSystem fs, MessageFilter v, final ProgressObserver observer) {

    Map<Long, Message> msgs = new HashMap<Long, Message>();

    final File[] files = listStoreFiles(fs, smsStore);
    if (files != null && files.length != 0) {
      Set<Long> finished = new HashSet<Long>();
      for (final int[] i = new int[]{0}; i[0] < files.length; i[0]++) {
        File file = files[i[0]];
        long delay = i[0] == (files.length - 1) ? 100 : 0;
        try {
          ProgressObserver _p = observer == null ? null : new ProgressObserver() {
            public void update(long current, long total) {
              int c = (int) (current);
              int t = (int) (total);
              observer.update((c * 100 / t / files.length) + (i[0] * 100 / files.length), 100);
            }
          };
          getMessages(file, fs, v, _p, msgs, finished, delay, 500);
          if (observer != null) {
            observer.update(100, 100);
          }
        } catch (AdminException e) {
          logger.error(e, e);
        }
      }
    }
    return msgs.values();
  }

  /**
   * Возвращает список сообщений из оперативного стораджа
   *
   * @param smscInstanceNumber номер инстанца СМСЦ, чей сторадж надо прочитать
   * @param filter             фильтр сообщений. Если null, то загружаются все сообщения
   * @param progressObserver   обзервер для отслеживания процесса завершения операции
   * @return список найденных сообщений
   * @throws AdminException если произошла ошибка при чтении стораджа
   */
  public Collection<Message> getMessages(int smscInstanceNumber, MessageFilter filter, ProgressObserver progressObserver) throws AdminException {
    if (smscInstanceNumber >= smsStorePaths.length)
      throw new OperativeStoreException("invalid_smsc_instance", String.valueOf(smscInstanceNumber));

    File smsStore = smsStorePaths[smscInstanceNumber];
    if (!smsStore.exists())
      return Collections.emptyList();

    return getMessages(smsStore, fs, filter, progressObserver);
  }


  /**
   * Отменяет доставку смс
   *
   * @param smses список идентификаторов смс
   * @throws ru.novosoft.smsc.admin.AdminException
   *          ошибка
   */
  public void cancelSMS(CCSms... smses) throws AdminException {
    if (smses == null) {
      return;
    }
    if (!cc.isOnline())
      throw new OperativeStoreException("cancel.sms.unavailable");

    cc.cancelSMS(smses);
  }

  private final static String CREATE_PROC_INSERT_RECORD = " create or replace procedure insert_rec( msg in SMS ) is begin ";
  private final static String INSERT_OP_SQL = "INSERT INTO ";
  private final static String INSERT_FIELDS =
      "  (id,st,submit_time,valid_time,attempts,last_result, " +
          "   last_try_time,next_try_time,oa,da,dda,mr,svc_type,dr,br,src_msc,     " +
          "   src_imsi,src_sme_n,dst_msc,dst_imsi,dst_sme_n,route_id,svc_id,prty,  " +
          "   src_sme_id,dst_sme_id,msg_ref,seq_num,arc,body_len) ";
  private final static String INSERT_VALUES =
      "	values (msg.id,msg.st,msg.submit_time,msg.valid_time,msg.attempts,msg.last_result, " +
          "   msg.last_try_time,msg.next_try_time,msg.oa,msg.da,msg.dda,msg.mr,msg.svc_type,msg.dr,msg.br,msg.src_msc," +
          "   msg.src_imsi,msg.src_sme_n,msg.dst_msc,msg.dst_imsi,msg.dst_sme_n,msg.route_id,msg.svc_id,msg.prty, " +
          "   msg.src_sme_id,msg.dst_sme_id, msg.msg_ref,msg.seq_num,msg.arc,msg.body_len); " +
          "end; ";


  private static final String oracleDriver = "oracle.jdbc.driver.OracleDriver";

  private final static String DELETE_OP_SQL = "TRUNCATE TABLE ";

  private final static String CALL_multinsert_sms_SQL = "{ call multinsert_sms(?) }"; //msgs in arraylist


  void clearTable(Connection conn, String tablePrefix) throws OperativeStoreException {
    try{
      PreparedStatement clearStmt = conn.prepareStatement(DELETE_OP_SQL + tablePrefix);
      clearStmt.executeUpdate();
      conn.commit();
    }catch (SQLException e) {
      throw new OperativeStoreException("cant_clean", e);
    }
  }

  private oracle.jdbc.OracleConnection getOracleConnection(DBExportSettings export) throws OperativeStoreException{
    try{
      int m_maxStatements = 10;
      boolean m_implicitCachingEnabled = false;
      boolean m_explicitCachingEnabled = false;

      Class.forName(oracleDriver);

      Properties props = new Properties();
      props.put("password", export.getPass());
      props.put("user", export.getUser());

      oracle.jdbc.OracleConnection con = (oracle.jdbc.OracleConnection) DriverManager.getConnection(export.getSource(), props);
      con.setStatementCacheSize(m_maxStatements);
      con.setImplicitCachingEnabled(m_implicitCachingEnabled);
      con.setExplicitCachingEnabled(m_explicitCachingEnabled);
      con.setAutoCommit(false);
      return con;
    } catch (Exception e) {
      throw new OperativeStoreException("cant_connect", e);
    }
  }

  private final Lock lock = new ReentrantLock();


  private OracleCallableStatement createInsert(Connection conn, String tablesPrefix) throws OperativeStoreException{
    String CREATE_PROC_SQL = CREATE_PROC_INSERT_RECORD + INSERT_OP_SQL + tablesPrefix + INSERT_FIELDS + INSERT_VALUES;

    PreparedStatement createprocStmt = null;
    try {
      createprocStmt = conn.prepareStatement(CREATE_PROC_SQL);
      createprocStmt.executeUpdate();
      conn.commit();

      return (OracleCallableStatement) conn.prepareCall(CALL_multinsert_sms_SQL);
    } catch (Exception e) {
      throw new OperativeStoreException("cant_insert", e);
    } finally {
      closeStatement(createprocStmt);
    }
  }

  private ArrayDescriptor createArrayDescriptor(Connection conn) throws OperativeStoreException{
    try{
      conn.getTypeMap().put("SMS", Class.forName(SqlSms.class.getName()));
      return ArrayDescriptor.createDescriptor("ARRAYLIST", conn);
    }   catch (Exception e) {
      throw new OperativeStoreException("cant_insert", e);
    }
  }

  public void export(DBExportSettings export, final ProgressObserver observer) throws AdminException {

    Connection conn = null;
    OracleCallableStatement callinsertStmt = null;
    try {
      lock.lock();

      conn = getOracleConnection(export);
      if (logger.isDebugEnabled()) {
        logger.debug("Export sms locked");
      }

      final String tablesPrefix = export.getPrefix();
      clearTable(conn, tablesPrefix);

      int arraySize = 25000;
      SqlSms msgs[] = new SqlSms[arraySize];

      callinsertStmt = createInsert(conn, tablesPrefix);
      oracle.sql.ArrayDescriptor ad = createArrayDescriptor(conn);

      int i = 0;

      MessageFilter filter = new MessageFilter();
      filter.setMaxRowSize(100000);

      try {
        int j = 0;
        for (File smsStore : smsStorePaths) {
          if (!fs.exists(smsStore)) {
            continue;
          }
          Collection<Message> messages = getMessages(smsStore, fs, filter, new ProgressObserverImpl(observer, j, smsStorePaths.length));

          if (logger.isInfoEnabled()) {
            logger.info("Inserting " + messages.size() + " records");
          }
          for (Message aMsgsFull : messages) {
            msgs[i] = convert(aMsgsFull);
            i++;
            if (i == arraySize) {
              insert(callinsertStmt, ad, conn, msgs);
              i = 0;
            }
          }
          j++;
        }
        if (i > 0) {
          SqlSms[] tr = new SqlSms[i];
          System.arraycopy(msgs, 0, tr, 0, i);
          insert(callinsertStmt, ad, conn, tr);
        }
      } catch (Exception e) {
        throw new OperativeStoreException("cant_insert", e);
      }
    } catch (AdminException e) {
      logger.error(e, e);
      rollback(conn);
      throw e;
    } catch (Exception e) {
      logger.error(e, e);
      rollback(conn);
      throw new OperativeStoreException("internal_error", e);
    } finally {
      closeStatement(callinsertStmt);
      closeConnection(conn);
      lock.unlock();
      if (logger.isDebugEnabled()) {
        logger.debug("Export sms unlocked");
      }
    }
  }


  private static void closeConnection(Connection conn) {
    if (conn != null) {
      try {
        conn.close();
      } catch (Exception e) {
        logger.warn("Couldn't close connection");
      }
    }
  }

  private static class ProgressObserverImpl implements ProgressObserver {

    private final ProgressObserver base;

    private final int inst;
    private final int numb;

    private ProgressObserverImpl(ProgressObserver base, int inst, int numb) {
      this.base = base;
      this.inst = inst;
      this.numb = numb;
    }

    public void update(long current, long total) {
      int c = (int) current;
      int t = (int) total;
      base.update((c * 100 / t / numb) + (inst * 100 / numb), 100);
    }
  }

  private void insert(OracleCallableStatement callinsertStmt, ArrayDescriptor ad, Connection conn, SqlSms[] msgs) throws SQLException {
    oracle.sql.ARRAY msgs1 = new oracle.sql.ARRAY(ad, conn, msgs);
    callinsertStmt.setARRAY(1, msgs1);
    callinsertStmt.execute();
    conn.commit();
  }

  private void rollback(Connection conn) {
    if (conn != null) {
      try {
        conn.rollback();
      } catch (SQLException ignored) {
      }
    }
  }

  private static SqlSms convert(Message m) {
    java.sql.Date submitTime = new java.sql.Date(m.getSubmitTime().getTime());

    long validTime = m.getValidTime() == null ? 0 : m.getValidTime().getTime();
    long lastTryTime = m.getLastTryTime() == null ? 0 : m.getLastTryTime().getTime();
    long nextTryTime = m.getNextTryTime() == null ? 0 : m.getNextTryTime().getTime();
    int attempts = m.getAttempts() == null ? 0 : m.getAttempts();

    int lastResult = m.getLastResult() == null ? 0 : m.getLastResult();

    Address origMask = m.getOriginatingAddress();

    Address destMask = m.getDestinationAddress();

    Address dDestMask = m.getDealiasedDestinationAddress();

    int mr = m.getMessageReference() == null ? 0 : m.getMessageReference();
    String sctype = m.getServiceType();
    short dreport = m.getDeliveryReport() == null ? 0 : m.getDeliveryReport().shortValue();
    short billr = m.getBillingRecord() == null ? 0 : m.getBillingRecord().shortValue();
    String odMsc = m.getOriginatingDescriptor() == null ? null : m.getOriginatingDescriptor().getMsc();
    String odImsi = m.getOriginatingDescriptor() == null ? null : m.getOriginatingDescriptor().getImsi();
    int odSme = m.getOriginatingDescriptor() == null ? 0 : m.getOriginatingDescriptor().getSme();
    String ddMsc = m.getDestinationDescriptor() == null ? null : m.getDestinationDescriptor().getMsc();
    String ddImsi = m.getDestinationDescriptor() == null ? null : m.getDestinationDescriptor().getImsi();
    int ddSme = m.getDestinationDescriptor() == null ? 0 : m.getDestinationDescriptor().getSme();

    String routeId = m.getRouteId();

    int serviceId = m.getServiceId() == null ? 0 : m.getServiceId();
    int priority = m.getPriority() == null ? 0 : m.getPriority();

    String srcSmeId = m.getSrcSmeId();
    String dstSmeId = m.getDstSmeId();


    short concatMsgRef = 0;

    short concatSeqNum = 0;


    byte arc = m.getArc();

    int bodyLen = m.getBody().length;

    return new SqlSms("SMS", m.getId(), m.getStatus().getCode(), submitTime, new java.sql.Date(validTime * 1000), attempts,
        lastResult, new java.sql.Date(lastTryTime * 1000), new java.sql.Date(nextTryTime * 1000), origMask.getSimpleAddress(), destMask.getSimpleAddress(),
        dDestMask.getSimpleAddress(), mr, sctype, dreport,
        billr, odMsc, odImsi, odSme, ddMsc,
        ddImsi, ddSme, routeId, serviceId, priority,
        srcSmeId, dstSmeId, concatMsgRef, concatSeqNum, arc, bodyLen);
  }


  private static void closeStatement(PreparedStatement stmt) {
    if (stmt != null)
      try {
        stmt.close();
      } catch (Exception e) {
        logger.warn("Couldn't close statement");
      }
  }
}
