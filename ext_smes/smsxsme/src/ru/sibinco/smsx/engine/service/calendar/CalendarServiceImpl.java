package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarHandleReceiptCmd;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.engine.service.calendar.datasource.DBCalendarDataSource;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.advertising.AdvertisingClientFactory;

import java.io.File;
import java.util.Calendar;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

public class CalendarServiceImpl implements CalendarService {

  private final CalendarProcessor processor;
  private final CalendarEngine engine;
  private final CalendarDataSource dataSource;
  private final AdvertisingClient advClient;

  public CalendarServiceImpl(XmlConfig config, final OutgoingQueue outQueue) {
    try {

      dataSource = new DBCalendarDataSource();

      advClient = AdvertisingClientFactory.createAdvertisingClient();

      MessagesQueue messagesQueue = new MessagesQueue();

      XmlConfigSection cal = config.getSection("calendar");

      engine = new CalendarEngine(outQueue, messagesQueue, dataSource, advClient, cal.getLong("engine.working.interval", 60000));
      engine.setAdvDelim(cal.getString("advertising.delimiter"));
      engine.setAdvSize(cal.getInt("advertising.size"));
      engine.setAdvService(cal.getString("advertising.service"));

      processor = new CalendarProcessor(messagesQueue, dataSource, cal.getInt("send.date.max.year", getCurrentYear() + 1));

    } catch (Throwable e) {
      throw new ServiceInitializationException(e);
    }
  }

  private static int getCurrentYear() {
    return Calendar.getInstance().get(Calendar.YEAR);
  }

  public long execute(CalendarSendMessageCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public int execute(CalendarCheckMessageStatusCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public boolean execute(CalendarHandleReceiptCmd cmd) throws CommandExecutionException {
    return processor.execute(cmd);
  }

  public void startService() {
    engine.start();
  }

  public void stopService() {
    engine.stop();
    advClient.close();
    dataSource.release();
  }

  public Object getMBean(String domain) {
    return new CalendarMBean(engine);
  }


}
