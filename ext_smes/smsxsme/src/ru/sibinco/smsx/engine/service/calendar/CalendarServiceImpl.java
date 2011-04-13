package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfigSection;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarHandleReceiptCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.engine.service.calendar.datasource.DBCalendarDataSource;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.advertising.AdvertisingClientFactory;

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
  private final DataSourceCleaner dsCleaner;

  public CalendarServiceImpl(XmlConfigSection cal, final OutgoingQueue outQueue, int serviceId) {
    try {

      dataSource = new DBCalendarDataSource();

      advClient = AdvertisingClientFactory.createAdvertisingClient();

      MessagesQueue messagesQueue = new MessagesQueue();

      engine = new CalendarEngine(outQueue, messagesQueue, dataSource, advClient, cal.getLong("engine.working.interval", 60000), serviceId);
      engine.setAdvDelim(cal.getString("advertising.delimiter"));
      engine.setAdvSize(cal.getInt("advertising.size"));
      engine.setAdvService(cal.getString("advertising.service"));

      processor = new CalendarProcessor(messagesQueue, dataSource, cal.getInt("send.date.max.year", getCurrentYear() + 1), serviceId);

      dsCleaner = new DataSourceCleaner(dataSource, cal.getInt("ds.clean.interval", 60000), cal.getInt("ds.clean.limit", 100), cal.getInt("ds.clean.maxAliveDays", 30));

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
    dsCleaner.start();
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
