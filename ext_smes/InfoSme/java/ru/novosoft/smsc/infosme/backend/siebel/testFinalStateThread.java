package ru.novosoft.smsc.infosme.backend.siebel;

import org.apache.log4j.Category;

import java.util.Date;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: bukind
 * Date: 02.09.2009
 * Time: 12:33:36
 * To change this template use File | Settings | File Templates.
 */

public class testFinalStateThread {

  private class TestSiebelDataProvider implements SiebelDataProvider {

    private Category log_;

    TestSiebelDataProvider(Category logger) {
      this.log_ = logger;
    }

    public SiebelMessage getMessage(String clcId) throws SiebelException {
      throw notImpl("getMessage");
    }

    public ResultSet getMessages(String waveId) throws SiebelException {
      throw notImpl("getMessages");
    }

    public void setMessageState(String clcId, SiebelMessage.State state) throws SiebelException {
      throw notImpl("setMessageState");
    }

    public SiebelMessage.State getMessageState(String clcId) throws SiebelException {
      throw notImpl("getMessageState");
    }

    /**
     * @param deliveryStates - map, where key is String with message Id, value is instance of SiebelMessage.DeliveryState
     * @throws SiebelException
     */
    public void updateDeliveryStates(Map deliveryStates) throws SiebelException {
      log_.info("updateDeliveryStates, size=" + deliveryStates.size());
      for (Iterator i = deliveryStates.entrySet().iterator(); i.hasNext();) {
        Map.Entry entry = (Map.Entry) i.next();
        String key = (String) entry.getKey();
        SiebelMessage.DeliveryState state = (SiebelMessage.DeliveryState) entry.getValue();
        log_.info("message " + key + " has state: " + state.getState().toString() + ", smpp:" + state.getSmppCode());
      }
    }

    public SiebelTask getTask(String waveId) throws SiebelException {
      throw notImpl("getTask");
    }

    public ResultSet getTasks(Date fromUpdate) throws SiebelException {
      throw notImpl("getTasks");
    }

    public ResultSet getTasksToUpdate() throws SiebelException {
      throw notImpl("getTasksFromTill");
    }

    public ResultSet getTasks() throws SiebelException {
      throw notImpl("getTasks");
    }

    public void setTaskStatus(String waveId, SiebelTask.Status status) throws SiebelException {
      log_.info("wave " + waveId + "has state:" + status.toString());
    }

    public SiebelTask.Status getTaskStatus(String waveId) throws SiebelException {
      throw notImpl("getTaskStatus");
    }

    public boolean containsUnfinished(String waveId) throws SiebelException {
      return false;  //To change body of implemented methods use File | Settings | File Templates.
    }

    public void connect(Properties props) throws SiebelException {
      throw notImpl("connect");
    }

    public boolean isShutdowned() {
      return false;
    }

    public void shutdown() {
    }

    private SiebelException notImpl(String what) {
      return new SiebelException(what + " not implemented");
    }
  }

  public static void main(String[] argv) {
    new testFinalStateThread().run(argv);
  }

  void run(String[] argv) {
    Category logger = Category.getInstance("main");
    SiebelDataProvider provider = new TestSiebelDataProvider(logger);

    try {
      SiebelFinalStateThread stateThread = new SiebelFinalStateThread("working", "processed", provider);
      stateThread.start();
      // stateThread.join(); // waiting until it is finished
    } catch (Exception e) {
      e.printStackTrace(System.out);
    }
  }

}
