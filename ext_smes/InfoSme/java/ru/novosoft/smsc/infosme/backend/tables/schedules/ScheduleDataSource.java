package ru.novosoft.smsc.infosme.backend.tables.schedules;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.util.Iterator;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 12:48:07 PM
 */
public class ScheduleDataSource extends AbstractDataSourceImpl
{
  public static final String SCHEDULES_PREFIX = "InfoSme.Schedules";
  private Category logger = Category.getInstance(this.getClass());

  public ScheduleDataSource()
  {
    super(new String[]{"name", "execute", "startDateTime"});
  }

  public QueryResultSet query(Config config, ScheduleQuery query_to_run)
  {
    clear();
    for (Iterator i = config.getSectionChildShortSectionNames(SCHEDULES_PREFIX).iterator(); i.hasNext();) {
      String scheduleName = (String) i.next();
      final String currentSchedulePrefix = SCHEDULES_PREFIX + '.' + StringEncoderDecoder.encodeDot(scheduleName);
      try {
        add(new ScheduleDataItem(scheduleName,
                                 config.getString(currentSchedulePrefix + ".execute"),
                                 config.getString(currentSchedulePrefix + ".startDateTime")));
      } catch (Exception e) {
        logger.error("Couldn't get parameter for schedule \"" + scheduleName + "\", schedule skipped", e);
      }
    }
    return super.query(query_to_run);
  }
}
