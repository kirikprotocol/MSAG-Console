package ru.novosoft.smsc.infosme.backend.tables.schedules;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.config.schedules.Schedule;

import java.util.Iterator;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 12:48:07 PM
 */
public class ScheduleDataSource extends AbstractDataSourceImpl
{
  private Category logger = Category.getInstance(this.getClass());

  public ScheduleDataSource()
  {
    super(new String[]{"name", "execute", "startDateTime"});
  }

  public QueryResultSet query(InfoSmeConfig config, ScheduleQuery query_to_run)
  {
    clear();

    for (Iterator i = config.getSchedules().iterator(); i.hasNext();) {
      Schedule s = (Schedule)i.next();
      try {
        add(new ScheduleDataItem(s.getId(),
                                 s.getExecuteStr(), s.getStartDateTimeStr()));
      } catch (Exception e) {
        logger.error("Couldn't get parameter for schedule \"" + s.getId() + "\", schedule skipped", e);
      }
    }
    return super.query(query_to_run);
  }
}
