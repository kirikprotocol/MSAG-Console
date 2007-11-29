package ru.sibinco.smpp.ub_sme;

import java.util.*;


/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 6, 2006
 * Time: 1:04:28 PM
 */
public class MessageStatusChangedNotifier extends Thread {

    private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(MessageStatusChangedNotifier.class);

    private boolean started = false;
    private long pollingInterval = 10000L;
    private Object monitor = new Object();
    private Object shutmonitor = new Object();

    private Map listeners = new HashMap();
    private List notifications = new LinkedList();

    public MessageStatusChangedNotifier() {
        setName("MessageStatusChangedNotifier");
        if (Logger.isInfoEnabled()) Logger.info(getName() + " created.");
    }

    protected boolean registerMessageStatusListener(MessageStatusListener listener) {
        return listeners.put(listener.getName(), listener) == null;
    }

    protected synchronized void notifyListener(String listenerName, int messageId, int status) {
        if (listenerName != null && messageId != -1)
            notifications.add(new Notification(listenerName, messageId, status));
    }

    public void startService() {
        started = true;
        if (Logger.isInfoEnabled()) Logger.info(getName() + " started.");
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
        while (true) {
            while (!notifications.isEmpty()) {
                Notification n;
                synchronized (notifications) {
                    n = (Notification) notifications.remove(0);
                }
                MessageStatusListener listener = (MessageStatusListener) listeners.get(n.getListenerName());
                if (listener == null) {
                    if (Logger.isInfoEnabled())
                        Logger.info(getName() + " could not find abonentRequest status listener: " + n.getListenerName());
                } else {
                    listener.statusChanged(n.getMessageId(), n.getStatus());
                    if (Logger.isDebugEnabled())
                        Logger.debug(getName() + " notified abonentRequest status listener: " + n.getListenerName() + ". MsgID=" + n.getMessageId() + "; Status=" + n.getStatus());
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


    class Notification {
        private String listenerName;
        private int messageId;
        private int status;

        Notification(String listenerName, int messageId, int status) {
            this.listenerName = listenerName;
            this.messageId = messageId;
            this.status = status;
        }

        public String getListenerName() {
            return listenerName;
        }

        public int getMessageId() {
            return messageId;
        }

        public int getStatus() {
            return status;
        }
    }
}
