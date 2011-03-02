package mobi.eyeline.informer.admin.notifications;

import com.eyeline.utils.ThreadFactoryWithCounter;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListenerStub;
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
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.11.2010
 * Time: 13:46:28
 */
public class DeliveryNotificationsProvider extends DeliveryChangeListenerStub {
  private final Logger log = Logger.getLogger("NOTIFICATION_DAEMON");

  private final DeliveryNotificationsContext context;
  private final Map<String, AggregatedEmailNotificationTask> userEmailNotifications = new HashMap<String, AggregatedEmailNotificationTask>();

  private final Object lock = new Object();

  private NotificationSettings notificationSettings;

  private final ScheduledThreadPoolExecutor scheduler;
  private static final long AGGREGATE_TIME = 60;
  private static final long SHUTDOWN_WAIT_TIME = 120;
  private static final int POOL_SIZE = 1;
  private static final int MAX_QUEUE_SIZE = 10000;


  public DeliveryNotificationsProvider(DeliveryNotificationsContext context, NotificationSettings notificationSettings) {
    this.context = context;
    this.notificationSettings = notificationSettings;
    scheduler = new ScheduledThreadPoolExecutor(POOL_SIZE, new ThreadFactoryWithCounter("DeliveryNotificationDaemon-"));
    context.getDeliveryChangesDetector().addListener(this);

  }

  public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
    processDeliveryNotification(e);
  }

  public void updateSettings(NotificationSettings settings) throws AdminException {
    settings.validate();
    this.notificationSettings = settings;
  }


  public void processDeliveryNotification(ChangeDeliveryStatusEvent notification) {
        try {
          User user = context.getUser(notification.getUserId());
          if (user != null) {
            Delivery delivery = context.getDelivery(user.getLogin(), notification.getDeliveryId());
            String sAddr = delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS);
            if (sAddr != null && sAddr.length() > 0) {
              Address smsNotificationAddress = new Address(sAddr);
              SMSNotificationTask task = new SMSNotificationTask(smsNotificationAddress, user, notification, delivery.getName());
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
                  scheduleOrWait(notificationTask, AGGREGATE_TIME, TimeUnit.SECONDS);
                }
                notificationTask.addNotification(notification, delivery.getName());
              }
            }
          }
        }
        catch (Exception e) {
          log.error("error ", e);
        }
  }



  private void scheduleOrWait(Callable<Object> task, long time, TimeUnit units) {
    while (scheduler.getTaskCount() - scheduler.getCompletedTaskCount() >= MAX_QUEUE_SIZE) {
      try {
        lock.wait(100);
      }
      catch (InterruptedException e) {
      }
    }
    scheduler.schedule(task, time, units);
  }


  public void shutdown() {
    try {
      this.context.getDeliveryChangesDetector().removeListener(this);
      scheduler.setExecuteExistingDelayedTasksAfterShutdownPolicy(true);
      scheduler.shutdown();
      try {
        scheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.SECONDS);
      }
      catch (InterruptedException e) {
        scheduler.shutdownNow();
      }
    } catch (Exception ignored) {}
  }

  private static String formatTemplate(String template, ChangeDeliveryStatusEvent n, String deliveryName, User user) {
    return MessageFormat.format(
          template,
          deliveryName,
          new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(n.getEventDate()),
          user.getFirstName() + " " + user.getLastName()
      );
  }

  public void sendTestEmailNotification(User user, String toEmail, NotificationSettings settings) throws AdminException {
    try {
      AggregatedEmailNotificationTask task = new AggregatedEmailNotificationTask(toEmail, user, settings);
      task.addNotification(new ChangeDeliveryStatusEvent(DeliveryStatus.Active,new Date(),1,user.getLogin())
      ,"EmailTestActivate");
      task.addNotification(new ChangeDeliveryStatusEvent(DeliveryStatus.Finished,new Date(),2,user.getLogin())
      ,"EmailTestFinished");
      task.call();
    }
    catch (Exception e) {
      throw new DeliveryNotificationException("email.test.fail",e,e.getMessage());
    }
  }

  public void sendTestSMSNotification(User user, Address toAddress, DeliveryStatus status, NotificationSettings settings) throws AdminException {
    try {
      ChangeDeliveryStatusEvent event = new ChangeDeliveryStatusEvent(status,new Date(),1,user.getLogin());
      SMSNotificationTask task = new SMSNotificationTask(toAddress,user,event,
          status==DeliveryStatus.Active ? "SmsTestActivate":"SmsTestFinished", settings);
      task.call();
    }
    catch (Exception e) {
      throw new DeliveryNotificationException("sms.test.fail",e,e.getMessage());
    }
  }

  /*======================= */

  class SMSNotificationTask implements Callable<Object> {
    final Address address;

    private final User user;
    private final ChangeDeliveryStatusEvent notification;
    private final String deliveryName;
    private NotificationSettings settings;


    public SMSNotificationTask(Address address, User user, ChangeDeliveryStatusEvent notification, String deliveryName) {
      this(address, user, notification, deliveryName, notificationSettings);
    }
    public SMSNotificationTask(Address address, User user, ChangeDeliveryStatusEvent notification, String deliveryName, NotificationSettings settings) {
      this.address = address;
      this.user = user;
      this.notification = notification;
      this.deliveryName = deliveryName;
      this.settings = settings;
    }

    public Object call() throws Exception {
      try {
        TestSms testSms = TestSms.sms(false);
        testSms.setDestAddr(address);
        testSms.setSourceAddr(notificationSettings.getSmsSenderAddress());

        String template = notification.getStatus() == DeliveryStatus.Active ? settings.getSmsTemplateActivated() :
            settings.getSmsTemplateFinished();

        String text = formatTemplate(template, notification, deliveryName, user);
        testSms.setText(text);
        context.sendTestSms(testSms);
      }
      catch (Exception e) {
        log.error("Error sending notification sms to " + address, e);
      }
      return null;
    }
  }




  /*======================= */

  class AggregatedEmailNotificationTask implements Callable<Object> {
    private final String email;
    private final User user;
    private final LinkedList<String> notifications;
    private NotificationSettings settings;



    public AggregatedEmailNotificationTask(String email, User user) {
      this(email,user,notificationSettings);
    }

    AggregatedEmailNotificationTask(String email, User user,  NotificationSettings settings) {
      this.email = email;
      this.user = user;
      this.settings = settings;
      this.notifications = new LinkedList<String>();
    }

    public void addNotification(ChangeDeliveryStatusEvent n, String deliveryName) {

      String template = n.getStatus() == DeliveryStatus.Active ? settings.getEmailTemplateActivated() : settings.getEmailTemplateFinished();

      synchronized (notifications) {
        notifications.add(formatTemplate(template, n, deliveryName, user));
      }
    }

    public Object call() throws Exception {
      synchronized (lock) {
        userEmailNotifications.remove(email);
      }
      try {


        StringBuilder sb = new StringBuilder();
        for (String n : notifications)
          sb.append(n).append("\n");


        Session session = Session.getDefaultInstance(settings.getMailProperties());
        MimeMessage message = new MimeMessage(session);
        message.addRecipient(javax.mail.Message.RecipientType.TO, new InternetAddress(email));
        message.setSubject(settings.getEmailSubjectTemplate(), "UTF-8");
        message.setText(sb.toString(), "UTF-8");

        Transport transport = session.getTransport();
        transport.connect();
        transport.sendMessage(message, message.getRecipients(javax.mail.Message.RecipientType.TO));
        transport.close();

      }
      catch (Exception e) {
        log.error("Unable to send email notification to " + email, e);
      }
      return null;
    }
  }

}