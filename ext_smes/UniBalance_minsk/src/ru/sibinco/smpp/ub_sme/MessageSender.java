package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.*;


public class MessageSender extends Thread implements ProductivityControllable {

  private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(MessageSender.class);

  private Object sendMonitor = new Object();
  private boolean started = false;
  private long pollingInterval = 1000L;
  private OutgoingQueue outQueue = null;
  private Multiplexor multiplexor;
  private Object monitor = new Object();
  private Object shutmonitor = new Object();
  private long syncSendTimeout = 10000;
  private long sendDelay = 0;

  private long count;
  private boolean counterEnabled;
  private long counterStartTime;


  public MessageSender(OutgoingQueue outQueue, Multiplexor multiplexor, String name, long pollingInterval) {
    setName(name);
    if (pollingInterval > 1000)
      this.pollingInterval = pollingInterval;
    this.outQueue = outQueue;
    this.multiplexor = multiplexor;
    outQueue.setSendNotifyMonitor(monitor);
  }

  public void setSyncSendTimeout(long syncSendTimeout) {
    if (syncSendTimeout > 0 && syncSendTimeout < 60000)
      this.syncSendTimeout = syncSendTimeout;
    else
      Logger.warn("Illegal syncronized send timeout value=" + syncSendTimeout + ", use default: " + this.syncSendTimeout);
  }

  public void setSendDelay(long sendDelay) {
    if (sendDelay >= 0 && sendDelay <= 60000)
      this.sendDelay = sendDelay;
    else
      Logger.warn("Illegal send delay value=" + sendDelay + ", use default: " + this.sendDelay);
  }

  public void startService() {
    started = true;
    start();
  }

  public void stopService() {
    synchronized (shutmonitor) {
      synchronized (monitor) {
        started = false;
        monitor.notifyAll();
      }
      try {
        shutmonitor.wait();
      } catch (InterruptedException e) {
        Logger.warn(getName() + " shutting down exception:", e);
      }
    }
  }

  public boolean isStarted() {
    return started;
  }

  public void run() {
    OutgoingObject obj;
    while (true) {
      while ((obj = outQueue.getOutgoingObject()) != null && started) {
        try {
          Connector connector = null;
          Message msg = obj.getOutgoingMessage();
          // try to lookup connector by name from outgoing object
          connector = multiplexor.lookupConnectorByName(obj.getConnector());
          // if connector not defined try to lookup connector by name from outgoing message
          if (connector == null)
            connector = multiplexor.lookupConnectorByName(msg.getConnectionName());
          // if connector not defined try to lookup connector by source address of outgoing message
          if (connector == null) {
            String connectorName = msg.getSourceAddress();
            if (connectorName.lastIndexOf(":") > -1) {
              connectorName = connectorName.substring(connectorName.lastIndexOf(":") + 1);
            }
            connector = multiplexor.lookupConnectorByName(connectorName);
          }
          // if connector not defined try to lookup connector by destination address
          if (connector == null &&
              msg.getDestinationAddress() != null) {
            connector = multiplexor.lookupConnector(msg.getDestinationAddress());
          }
          // connector SHOULD be defined here !!!
          if (connector == null)
            throw new SMPPUserException(Data.ESME_RINVADR, "No route found for message: source #" + msg.getSourceAddress() + "; destination #" + msg.getDestinationAddress());
          if(msg.getSequenceNumber()==-1){
            multiplexor.assingSequenceNumber(msg, connector.getName());
          }
          outQueue.messageSubmitted(obj);
          if (Logger.isInfoEnabled())
            Logger.info("Sending message in async mode, address #" + msg.getSourceAddress() + "; abonent #" + msg.getDestinationAddress());
          multiplexor.sendMessage(msg, connector.getName(), false);
          count();
          if (sendDelay > 0) {
            synchronized (sendMonitor) {
              try {
                sendMonitor.wait(sendDelay);
              } catch (InterruptedException e) {
                Logger.warn(getName() + " was interrupted during send delay.", e);
              }
            }
          }
        } catch (SMPPNotConnectedException e) {
          Logger.warn(e.getMessage(), e);
        } catch (SMPPUserException e) {
          Logger.warn("exc: " + e.getErrorCode(), e);
        } catch (SMPPException e) {
          Logger.warn("exc:", e);
        }
      }
      synchronized (monitor) {
        if (!started) break;
        try {
          monitor.wait(pollingInterval);
        } catch (InterruptedException e) {
          if (Logger.isDebugEnabled()) Logger.debug(getName() + " was interrupted.", e);
        }
      }
    }
    synchronized (shutmonitor) {
      shutmonitor.notifyAll();
    }
  }

  public Object getSendMonitor() {
    return sendMonitor;
  }

  public long getEventsCount() {
    return count;
  }

  public synchronized void count() {
    count++;
  }

  public void setEventsCounterEnabled(boolean enabled) {
    counterEnabled = enabled;
    if (counterEnabled) {
      counterStartTime = System.currentTimeMillis();
    }
  }

  public synchronized void resetEventsCounter() {
    count = 0;
    counterStartTime = System.currentTimeMillis();
  }

  public long getCounterStartTime() {
    return counterStartTime;
  }

}
