package ru.sibinco.otasme.engine.service;

import com.eyeline.sme.utils.statemachine.AbstractState;
import com.eyeline.sme.utils.statemachine.EventSink;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.network.smpp.SMPPOutgoingQueue;
import ru.sibinco.otasme.network.smpp.SMPPTransportObject;
import ru.sibinco.smsc.utils.smscenters.SmsCenter;
import ru.sibinco.smsc.utils.smscenters.SmsCentersList;
import snaq.db.ConnectionPool;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * User: artem
 * Date: Oct 5, 2007
 */

abstract class AbstractSubscriptionState extends AbstractState implements SubscriptionState {

  protected static final Category log = Category.getInstance("STATE MASHINE");

  private final ConnectionPool pool;
  private final SMPPOutgoingQueue outQueue;
  private final SmsCentersList smsCenters;

  public AbstractSubscriptionState(EventSink eventSink, ConnectionPool pool, SMPPOutgoingQueue outQueue, SmsCentersList smsCenters) {
    super(eventSink);
    this.pool = pool;
    this.outQueue = outQueue;
    this.smsCenters = smsCenters;
  }

  protected abstract void doBeforeRemove(String abonentAddress);

  protected void addAbonentIntoDB(String abonentAddress) throws SQLException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(SmeProperties.Session.ADD_ABONENT_SQL);
      ps.setString(1, abonentAddress);
      ps.executeUpdate();

    } finally {
      close(null, ps, conn);
    }
  }

  protected void removeAbonentFromDB(String abonentAddress) throws SQLException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(SmeProperties.Session.DELETE_ABONENT_SQL);
      ps.setString(1, abonentAddress);
      ps.executeUpdate();

    } finally {
      close(null, ps, conn);
    }
  }

  protected boolean abonentPresentsInDB(String abonentAddress) throws SQLException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(SmeProperties.Session.FIND_ABONENT_SQL);
      ps.setString(1, abonentAddress);
      rs = ps.executeQuery();

      return rs.next();

    } finally {
      close(rs, ps, conn);
    }
  }

  private static void close(ResultSet rs, PreparedStatement ps, Connection conn) {
    try {
      if (rs != null)
        rs.close();
    } catch (SQLException e) {
      log.error(e, e);
    }
    try {
      if (ps != null)
        ps.close();
    } catch (SQLException e) {
      log.error(e, e);
    }
    try {
      if (conn != null)
        conn.close();
    } catch (SQLException e) {
      log.error(e, e);
    }
  }

  protected SmsCenter getSmscAddressForAbonent(String abonentAddress) {
    return smsCenters.getSmsCenterByAddress(abonentAddress);
  }

  protected void sendSRCommand(String wtsServiceName, String abonentNumber, int delay) {
    final Message otaRequest = new Message();
    otaRequest.setType(Message.TYPE_WTS_REQUEST);
    otaRequest.setWtsOperationCode(Message.WTS_OPERATION_CODE_COMMAND);
    otaRequest.setWTSUserId(removePlusFromAbonentNumber(abonentNumber));
    otaRequest.setWTSServiceName(wtsServiceName);
    otaRequest.setSourceAddress(SmeProperties.Session.OTA_NUMBER);
    otaRequest.setDestinationAddress(SmeProperties.Session.OTA_NUMBER);
    otaRequest.setWtsRequestReference(abonentNumber);

    final SMPPTransportObject outObj = new SMPPTransportObject();
    outObj.setOutgoingMessage(otaRequest);
    if (delay > 0)
      outObj.setNextTime(System.currentTimeMillis() + delay);
    outQueue.addOutgoingObject(outObj);
  }

  protected void sendSRCommand(String wtsServiceName, String abonentNumber) {
    sendSRCommand(wtsServiceName, abonentNumber, 0);
  }

  private static String removePlusFromAbonentNumber(String number) {
    return (number.indexOf("+") >= 0) ? number.substring(1) : number;
  }
}
