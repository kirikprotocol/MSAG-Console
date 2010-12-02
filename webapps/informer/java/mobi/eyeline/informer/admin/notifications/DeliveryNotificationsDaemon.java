package mobi.eyeline.informer.admin.notifications;

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
import java.util.*;
import java.util.concurrent.*;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.11.2010
 * Time: 13:46:28
 */
public class DeliveryNotificationsDaemon extends DeliveryChangeListenerStub {
  private final Logger log = Logger.getLogger("NOTIFICATION_DAEMON");

  private final DeliveryNotificationsContext context;
  private final Map<String, AggregatedEmailNotificationTask> userEmailNotifications = new HashMap<String, AggregatedEmailNotificationTask>();

  private final Object lock = new Object();

  private final ScheduledThreadPoolExecutor scheduler;
  private static final long AGGREGATE_TIME = 60;
  private static final long SHUTDOWN_WAIT_TIME = 120;
  private static final int POOL_SIZE = 1;
  private static final int MAX_QUEUE_SIZE = 10000;


  public DeliveryNotificationsDaemon(DeliveryNotificationsContext context) {      
    this.context = context;
    scheduler = new ScheduledThreadPoolExecutor(POOL_SIZE, new ThreadFactory() {
      int n = 0;

      public Thread newThread(Runnable runnable) {
        n++;
        return new Thread(runnable, "DeliveryNotificationDaemon-" + n);
      }
    });
  }

  public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
    processDeliveryNotification(e);
  }


  public void processDeliveryNotification(ChangeDeliveryStatusEvent notification) {
        try {
          User user = context.getUser(notification.getUserId());
          if (user != null) {
            Delivery delivery = context.getDelivery(user.getLogin(), user.getPassword(), notification.getDeliveryId());
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


  public void shutdown() throws InterruptedException {
    scheduler.setExecuteExistingDelayedTasksAfterShutdownPolicy(true);
    scheduler.shutdown();
    try {
      scheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.SECONDS);
    }
    catch (InterruptedException e) {
      scheduler.shutdownNow();
    }
  }

  private static String formatTemplate(String template, ChangeDeliveryStatusEvent n, String deliveryName, User user) {
    return MessageFormat.format(
          template,
          deliveryName,
          new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(n.getEventDate()),
          user.getFirstName() + " " + user.getLastName()
      );
  }

  public void sendTestEmailNotification(User user, String toEmail, Properties javaMailProps, Properties notificationTemplates) throws AdminException {
    try {
      AggregatedEmailNotificationTask task = new AggregatedEmailNotificationTask(toEmail, user, javaMailProps, notificationTemplates);
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

  public void sendTestSMSNotification(User user, Address toAddress, DeliveryStatus status, Properties notificationTemplates) throws AdminException {
    try {
      ChangeDeliveryStatusEvent event = new ChangeDeliveryStatusEvent(status,new Date(),1,user.getLogin());
      SMSNotificationTask task = new SMSNotificationTask(toAddress,user,event,status==DeliveryStatus.Active ? "SmsTestActivate":"SmsTestFinished",notificationTemplates);
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
    private Properties templates;


    public SMSNotificationTask(Address address, User user, ChangeDeliveryStatusEvent notification, String deliveryName) {
      this(address,user,notification,deliveryName,context.getNotificationTemplates());
    }

    public SMSNotificationTask(Address address, User user, ChangeDeliveryStatusEvent notification, String deliveryName, Properties templates) {
      this.address = address;
      this.user = user;
      this.notification = notification;
      this.deliveryName = deliveryName;
      this.templates = templates;
    }

    public Object call() throws Exception {
      try {
        TestSms testSms = new TestSms();
        testSms.setDestAddr(address);
        testSms.setSourceAddr(context.getSmsSenderAddress());
        testSms.setFlash(false);
        testSms.setMode(TestSms.Mode.SMS);

        String template = (String) templates.get(
            notification.getStatus() == DeliveryStatus.Active
                ?
                DeliveryNotificationTemplatesConstants.SMS_TEMPLATE_ACTIVATED
                :
                DeliveryNotificationTemplatesConstants.SMS_TEMPLATE_FINISHED
        );

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
    private Properties mailProps;
    private Properties templates;



    public AggregatedEmailNotificationTask(String email, User user) {
      this(email,user,context.getJavaMailProperties(),context.getNotificationTemplates());
    }

    AggregatedEmailNotificationTask(String email, User user,  Properties mailProps, Properties templates) {
      this.email = email;
      this.user = user;
      this.mailProps = mailProps;
      this.templates = templates;
      this.notifications = new LinkedList<String>();
    }

    public void addNotification(ChangeDeliveryStatusEvent n, String deliveryName) {

      String template = n.getStatus() == DeliveryStatus.Active
          ?
          templates.getProperty(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_ACTIVATED)
          :
          templates.getProperty(DeliveryNotificationTemplatesConstants.EMAIL_TEMPLATE_FINISHED);

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
        log.error("Unable to send email notification to " + email, e);
      }
      return null;
    }
  }

}