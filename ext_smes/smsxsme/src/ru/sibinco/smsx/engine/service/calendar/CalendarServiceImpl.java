package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarHandleReceiptCmd;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.engine.service.calendar.datasource.DBCalendarDataSource;

import java.io.File;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

class CalendarServiceImpl implements CalendarService {

  private final CalendarProcessor processor;
  private final CalendarEngine engine;
  private final MessagesQueue messagesQueue;
  private final CalendarDataSource dataSource;

  CalendarServiceImpl(XmlConfig config, final OutgoingQueue outQueue) {
    try {

      dataSource = new DBCalendarDataSource();

      messagesQueue = new MessagesQueue();

      engine = new CalendarEngine(outQueue, messagesQueue, dataSource, config.getSection("calendar").getLong("engine.working.interval"));

      processor = new CalendarProcessor(messagesQueue, dataSource, config.getSection("calendar").getInt("send.date.max.year"));

    } catch (Throwable e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void execute(CalendarSendMessageCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(CalendarCheckMessageStatusCmd cmd) {
    processor.execute(cmd);
  }

  public boolean execute(CalendarHandleReceiptCmd cmd) {
    return processor.execute(cmd);
  }

  public void startService() {
    engine.start();
  }

  public void stopService() {
    engine.stop();
    dataSource.release();
  }

  public Object getMBean(String domain) {
    return new CalendarMBean(engine);
  }


}
