package mobi.eyeline.informer.admin.notifications;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryNotificationsAdapter;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Callable;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.11.2010
 * Time: 13:46:28
 */
public class DeliveryNotificationsDaemon extends DeliveryNotificationsAdapter {
  private final Logger log = Logger.getLogger(this.getClass());

  private final AdminContext context;
  private final Map<String, AggregatedEmailNotificationTask> userEmailNotifications = new HashMap<String, AggregatedEmailNotificationTask>();

  private final Object lock = new Object();

  private final ScheduledThreadPoolExecutor scheduler;
  private static final long AGGREGATE_TIME = 60000L;
  private static final long SHUTDOWN_WAIT_TIME = 120000L;
  private static final int POOL_SIZE = 1;
  private static final int MAX_QUEUE_SIZE = 10000;


  public DeliveryNotificationsDaemon(AdminContext context) {
    this.context = context;
    scheduler = new ScheduledThreadPoolExecutor(POOL_SIZE, new ThreadFactory() {
      int n = 0;

      public Thread newThread(Runnable runnable) {
        n++;
        return new Thread(runnable, "DeliveryNotificationDaemon-" + n);
      }
    });
  }

  @Override
  public void onDeliveryStartNotification(DeliveryNotification notification) {
    processDeliveryNotification(notification);
  }

  @Override
  public void onDeliveryFinishNotification(DeliveryNotification notification) {
    processDeliveryNotification(notification);
  }

  public void processDeliveryNotification(DeliveryNotification notification) {
        try {
          User user = context.getUser(notification.getUserId());
          if (user != null) {
            Delivery delivery = context.getDelivery(user.getLogin(), user.getPassword(), notification.getDeliveryId());
            String sAddr = delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
            if (sAddr != null && sAddr.length() > 0) {
              Address smsNotificationAddress = new Address(sAddr);
              SMSNotificationTask task = new SMSNotificationTask(smsNotificationAddress, user, new DeliveryNotificationWrapper(notification, delivery.getName()));
              synchronized (lock) {
                scheduleOrWait(task, 0, TimeUnit.MILLISECONDS);
              }
            }

            String email = delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS);
            if (email != null && email.length() > 0) {
              synchronized (lock) {
                AggregatedEmailNotificationTask notificationTask = userEmailNotifications.get(email);
                if (notificationTask == null) {
                  notificationTask = new AggregatedEmailNotificationTask(email, user);
                  userEmailNotifications.put(email, notificationTask);
                  scheduleOrWait(notificationTask, AGGREGATE_TIME, TimeUnit.MICROSECONDS);
                }
                notificationTask.addNotification(new DeliveryNotificationWrapper(notification, delivery.getName()));
              }
            }
          }
        }
        catch (Exception e) {
          e.printStackTrace();
          log.error("error ", e);
        }
  }



  private void scheduleOrWait(Callable<Object> task, long time, TimeUnit units) {
    while (scheduler.getTaskCount() - scheduler.getCompletedTaskCount() >= MAX_QUEUE_SIZE) {
      try {
        lock.wait(100);
      }
      catch (InterruptedException e) {
        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      }
    }
    scheduler.schedule(task, time, units);
  }


  public void shutdown() throws InterruptedException {
    scheduler.setExecuteExistingDelayedTasksAfterShutdownPolicy(true);
    scheduler.shutdown();
    try {
      scheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.MILLISECONDS);
    }
    catch (InterruptedException e) {
      scheduler.shutdownNow();
    }
  }


  /*  ======================= */

  class SMSNotificationTask implements Callable<Object> {
    final Address address;

    private final User user;
    private final DeliveryNotificationWrapper notification;

    public SMSNotificationTask(Address address, User user, DeliveryNotificationWrapper notification) {
      this.address = address;
      this.user = user;
      this.notification = notification;
    }

    public Object call() throws Exception {
      try {
        TestSms testSms = new TestSms();
        testSms.setDestAddr(address);
        testSms.setSourceAddr(context.getSmsSenderAddress());
        testSms.setFlash(false);
        testSms.setMode(TestSms.Mode.SMS);

        String template = (String) context.getNotificationTemplates().get(
            notification.getType() == DeliveryNotificationType.DELIVERY_START
                ?
                DeliveryNotificationTemplatesConstants.SMS_TEMPLATE_ACTIVATED
                :
                DeliveryNotificationTemplatesConstants.SMS_TEMPLATE_FINISHED
        );

        String text = notification.formatTemplate(template, user);
        testSms.setText(text);
        context.sendTestSms(testSms);
      }
      catch (Exception e) {
        log.error("Error sending notification sms to " + address, e);
      }
      return null;
    }
  }

  /*  ======================= */

  class AggregatedEmailNotificationTask implements Callable<Object> {
    private final String email;
    private final User user;
    private final LinkedList<DeliveryNotificationWrapper> notifications;

    public AggregatedEmailNotificationTask(String email, User user) {
      this.email = email;
      this.user = user;
      this.notifications = new LinkedList<DeliveryNotificationWrapper>();
    }

    public void addNotification(DeliveryNotificationWrapper n) {
      synchronized (notifications) {
        notifications.add(n);
      }
    }

    public Object call() throws Exception {
      synchronized (lock) {
        userEmailNotifications.remove(email);
      }
      try {
        Properties templates = context.getNotificationTemplates();

        StringBuilder sb = new StringBuilder();
        for (DeliveryNotificationWrapper n : notifications) {
          String template = n.getType() == DeliveryNotificationType.DELIVERY_START
              ?
              templates.getProperty(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_ACTIVATED)
              :
              templates.getProperty(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_FINISHED);

          sb.append(n.formatTemplate(template, user)).append("\n");
        }


        Properties mailProps = context.getJavaMailProperties();
        Session session = Session.getDefaultInstance(mailProps);
        MimeMessage message = new MimeMessage(session);
        message.addRecipient(javax.mail.Message.RecipientType.TO, new InternetAddress(email));
        message.setSubject(templates.getProperty(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_SUBJECT), "UTF-8");
        message.setText(sb.toString(), "UTF-8");

        Transport transport = session.getTransport();
        transport.connect();
        transport.sendMessage(message, message.getRecipients(javax.mail.Message.RecipientType.TO));
        transport.close();

      }
      catch (Exception e) {
        log.error("unable to send email notification to " + email);
      }
      return null;
    }
  }

  /*=======================*/

  class DeliveryNotificationWrapper {
    private final DeliveryNotification notification;
    private final String deliveryName;

    public DeliveryNotificationWrapper(DeliveryNotification notification, String deliveryName) {
      this.deliveryName = deliveryName;
      this.notification = notification;
    }

    public String getDeliveryName() {
      return deliveryName;
    }

    public Date getEventDate() {
      return notification.getEventDate();
    }

    public int getDeliveryId() {
      return notification.getDeliveryId();
    }

    public String getUserId() {
      return notification.getUserId();
    }

    public DeliveryNotificationType getType() {
      return notification.getType();
    }

    public String formatTemplate(String template, User user) {
      return MessageFormat.format(
          template,
          getDeliveryName(),
          new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(getEventDate()),
          user.getFirstName() + " " + user.getLastName()
      );
    }
  }


}
