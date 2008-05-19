package ru.sibinco.smsx;

import org.apache.log4j.Category;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;
import ru.sibinco.smsx.utils.operators.OperatorsList;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

/**
 * User: artem
 * Date: 14.05.2008
 */

public class Context {

  private static final Category log = Category.getInstance("CONTEXT");

  private static Context instance;

  public static Context getInstance() {
    return instance;
  }

  private SmscTimezonesList timezonesList;
  private OperatorsList operatorsList;
  private ScheduledExecutorService configReloader;

  public static void init() {
    instance = new Context();

    instance.timezonesList = new SmscTimezonesList();
    try {
      instance.timezonesList.load("conf/timezones.xml", "conf/routes.xml");
    } catch (SmscTimezonesListException e) {
      throw new InitializationException(e);
    }

    instance.operatorsList = new OperatorsList("conf/operators.xml");

    instance.configReloader = Executors.newSingleThreadScheduledExecutor(new ThreadFactory(){
      public Thread newThread(Runnable r) {
        return new Thread(r, "ConfigReloadThread");
      }
    });

    instance.configReloader.scheduleWithFixedDelay(new Runnable() {
      public void run() {
        try {
          instance.timezonesList.load("conf/timezones.xml", "conf/routes.xml");
        } catch (Throwable e) {
          log.error("Can't reload timezones list", e);
        }

        instance.operatorsList.reloadOperators();
      }
    }, 600, 600, TimeUnit.SECONDS);
  }

  public void shutdown() {
    configReloader.shutdown();
  }

  public SmscTimezonesList getTimezones() {
    return timezonesList;
  }

  public OperatorsList getOperators() {
    return operatorsList;
  }
}
