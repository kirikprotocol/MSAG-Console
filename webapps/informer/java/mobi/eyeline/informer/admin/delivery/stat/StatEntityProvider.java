package mobi.eyeline.informer.admin.delivery.stat;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Functions;
import org.apache.log4j.Logger;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
 * @author Aleksandr Khalitov
 */
public abstract class StatEntityProvider {

  protected static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");

  protected static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

  protected static final Logger logger = Logger.getLogger(UserStatProvider.class);


  public abstract void visitEntities(Date from, Date till, EntityVisitor visitor) throws AdminException;

  public abstract void dropEntities(Date from, Date till) throws AdminException;


  protected static Date prepareDateForFilesLookup(Date date) {
    if (date == null)
      return null;
    date = Functions.convertTime(date, LOCAL_TIMEZONE, STAT_TIMEZONE);
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    c.set(Calendar.MINUTE, 0);
    c.set(Calendar.SECOND, 0);
    c.set(Calendar.MILLISECOND, 0);
    return c.getTime();
  }

  protected static Date prepareDateForEntitiesView(Date date) {
    if (date == null)
      return null;
    date = Functions.convertTime(date, STAT_TIMEZONE, LOCAL_TIMEZONE);
    Calendar c = Calendar.getInstance();
    c.setTime(date);
    return c.getTime();
  }

  public interface EntityVisitor {
    public boolean visit(StatEntity entity) throws AdminException;
  }
}
