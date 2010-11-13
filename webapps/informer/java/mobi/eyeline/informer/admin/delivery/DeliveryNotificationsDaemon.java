package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;


import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Callable;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 13.11.2010
 * Time: 13:46:28
 */
public class DeliveryNotificationsDaemon implements DeliveryNotificationsListener {
  Logger log = Logger.getLogger(this.getClass());

  AdminContext context;
  final Map<String, AggregatedEmailNotificationTask> userEmailNotifications = new HashMap<String, AggregatedEmailNotificationTask>();
  final List<Callable<Object>> tasks  = new LinkedList<Callable<Object>>();
  final Object lock = new Object();
  ScheduledExecutorService scheduler;
  private static final long AGGREGATE_TIME_SEC = 60;
  private static final long TERMINATION_WAIT_TIME_SEC = 120;

  public DeliveryNotificationsDaemon(AdminContext context) {
    this.context = context;
    // todo init scheduler
  }

  public void onDeliveryNotification(DeliveryNotification notification) {
    switch(notification.getType()) {
      case DELIVERY_START:
      case DELIVERY_FINISHED:
        User user = context.getUser(notification.getUserId());
        if(user!=null) {
          if(user.isEmailNotification()) {
            synchronized (lock) {
              AggregatedEmailNotificationTask notificationTask = userEmailNotifications.get(user.getLogin());
              if(notificationTask!=null) {
                notificationTask = new AggregatedEmailNotificationTask(user);
                tasks.add(notificationTask);
                userEmailNotifications.put(user.getLogin(),notificationTask);
                scheduler.schedule(notificationTask,AGGREGATE_TIME_SEC, TimeUnit.MILLISECONDS);
              }
              notificationTask.addNotification(notification);
            }
          }
          if(user.isSmsNotification()) {
            SMSNotificationTask task = new SMSNotificationTask(user,notification);
            synchronized (lock) {
              tasks.add(task);
              scheduler.schedule(task,0, TimeUnit.MILLISECONDS);
            }

          }
        }
        break;
      default:return;
    }
  }

  public void stop() throws InterruptedException {
    scheduler.shutdown();
    scheduler.awaitTermination(TERMINATION_WAIT_TIME_SEC,TimeUnit.SECONDS);
    //execute ones which not started
    for(Callable<Object> task : tasks) {

    }
  }

  private void taskStarted(User user,Callable<Object> task) {
    synchronized (lock) {
        tasks.remove(task);
        userEmailNotifications.remove(user.getLogin());
    }
  }


  class SMSNotificationTask implements Callable<Object> {
    private Date date;
    private User user;
    private DeliveryNotification notification;

    public SMSNotificationTask(User user, DeliveryNotification notification) {
      this.date = new Date();
      this.user = user;
      this.notification = notification;
    }

    public Object call() throws Exception {
      taskStarted(user,this);
      try {
        TestSms testSms  = new TestSms();
        testSms.setDestAddr(new Address(user.getPhone()));
        testSms.setSourceAddr(context.getSmsSenderAddress());
        testSms.setFlash(false);
        testSms.setMode(TestSms.Mode.SMS);
        Delivery d = context.getDelivery(user.getLogin(),user.getPassword(),notification.getDeliveryId());
        String template = (String) context.getNotificationTemplates().get("delivery.state.changed.sms.template");
        String text = MessageFormat.format(template,
            d.getName(),
            notification.getType()==DeliveryNotificationType.DELIVERY_START ? "Started":"",
            new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(notification.getEventDate()),
            user.getFirstName()+" "+user.getLastName()
        );
        testSms.setText(text);
        context.sendTestSms(testSms);
      }
      catch (Exception e) {
        log.error("Error sending notification sms to "+user.getSourceAddr(),e);
      }
      return null;
    }
  }


  class AggregatedEmailNotificationTask implements Callable<Object> {
    private Date date;
    private User user;
    private LinkedList<DeliveryNotification> notifications;

    public AggregatedEmailNotificationTask(User user) {
      this.date = new Date();
      this.user = user;
      this.notifications = new LinkedList<DeliveryNotification>();
    }

    public void addNotification(DeliveryNotification n) {
      synchronized (notifications) {
        notifications.add(n);
      }
    }

    public Object call() throws Exception {
      taskStarted(user,this);

      //todo send email
      
      return null;
    }
  }




}
