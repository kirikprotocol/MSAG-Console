package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.util.ConnectionPool;

import java.util.Iterator;
import java.util.Properties;
import java.util.HashMap;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.ResultSet;
import java.io.*;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 15:07:55
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManager
{
  private static ProfileManager instance = null;
  private static Object syncObj = new Object();

  public static ProfileManager getInstance() throws ScenarioInitializationException
  {
    synchronized(syncObj) {
      return (instance == null) ? (instance = new ProfileManager()):instance;
    }
  }

  private static Category logger = Category.getInstance(ProfileManager.class);

  private ProfileInfo DEFAULT_PROFILE_INFO = new ProfileInfo();
  private HashMap informFormats = new HashMap(); // TODO: synchronize it ???
  private HashMap notifyFormats = new HashMap(); // TODO: synchronize it ???

  private final static String DEFAULT_INFORM_ID = "defaultInformId";
  private final static String DEFAULT_NOTIFY_ID = "defaultNotifyId";
  private final static String INFORM_TEMPLATE_PREFIX = "informTemplate.";
  private final static String NOTIFY_TEMPLATE_PREFIX = "notifyTemplate.";

  private ProfileManager() throws ScenarioInitializationException
  {
    InputStream is = this.getClass().getClassLoader().getResourceAsStream(Constants.MCI_PROF_MTF_FILE);
    if (is == null)
      throw new ScenarioInitializationException("Failed to locate template properties file");

    Properties properties = new Properties();
    try {
      properties.load(is);
    } catch(IOException e) {
      throw new ScenarioInitializationException("Failed to load template properties", e);
    } finally {
      try { is.close(); } catch (Throwable th) {}
    }

    int defaultInformId = -1; int defaultNotifyId = -1;
    try {
      defaultInformId = Integer.parseInt(properties.getProperty(DEFAULT_INFORM_ID));
      defaultNotifyId = Integer.parseInt(properties.getProperty(DEFAULT_NOTIFY_ID));
    } catch (Exception e) {
      throw new ScenarioInitializationException("Failed to init default message template(s)", e);
    }

    for (Iterator i = properties.keySet().iterator(); i.hasNext();)
    {
      String key = (String)i.next();
      try
      {
        if (key.startsWith(INFORM_TEMPLATE_PREFIX))
        {
          int pos = key.indexOf('.'); if (pos <= 0) continue;
          int id = Integer.parseInt(key.substring(pos+1));
          FormatType template = new FormatType(id, properties.getProperty(key));
          if (id == defaultInformId) DEFAULT_PROFILE_INFO.informFormat = template;
          informFormats.put(new Long(id), template);
        }
        else if (key.startsWith(NOTIFY_TEMPLATE_PREFIX))
        {
          int pos = key.indexOf('.'); if (pos <= 0) continue;
          int id = Integer.parseInt(key.substring(pos+1));
          FormatType template = new FormatType(id, properties.getProperty(key));
          if (id == defaultNotifyId) DEFAULT_PROFILE_INFO.notifyFormat = template;
          notifyFormats.put(new Long(id), template);
        }
      }
      catch (Exception e) {
        throw new ScenarioInitializationException("Properties file is incorrect", e);
      }
    }

  }

  private FormatType getFormatType(long id, boolean inform)
  {
    Long key = new Long(id);
    FormatType formatType = (FormatType)((inform ? informFormats:notifyFormats).get(key));
    if (formatType != null) return formatType;
    return (inform ? DEFAULT_PROFILE_INFO.informFormat : DEFAULT_PROFILE_INFO.notifyFormat);
  }

  private final static String GET_PROFILE_SQL =
      "SELECT inform, notify, inform_id, notify_id, event_mask FROM mcisme_abonents WHERE abonent=?";
  private final static String SET_PROFILE_SQL =
      "UPDATE mcisme_abonents SET inform=?, notify=?, inform_id=?, notify_id=?, event_mask=? WHERE abonent=?";
  private final static String INS_PROFILE_SQL =
      "INSERT INTO mcisme_abonents (abonent, inform, notify, inform_id, notify_id, event_mask) VALUES (?, ?, ?, ?, ?, ?)";
  private final static String DEL_PROFILE_SQL =
      "DELETE FROM mcisme_abonents WHERE abonent=?";

  public ProfileInfo getProfileInfo(String abonent) throws ProfileManagerException
  {
    Connection connection  = null;
    PreparedStatement stmt = null;
    ResultSet rs = null;
    try
    {
      connection = ConnectionPool.getConnection();
      stmt = connection.prepareStatement(GET_PROFILE_SQL);
      stmt.setString(1, abonent);
      rs = stmt.executeQuery();
      if (rs == null || !rs.next()) return DEFAULT_PROFILE_INFO;

      ProfileInfo info = new ProfileInfo();
      int pos = 1;
      String result = rs.getString(pos++);
      info.inform = !(rs.wasNull() || result == null || result.length() <= 0 || result.trim().equals("N"));
      result = rs.getString(pos++);
      info.notify = !(rs.wasNull() || result == null || result.length() <= 0 || result.trim().equals("N"));
      long informId = rs.getLong(pos++); if (rs.wasNull()) informId = DEFAULT_PROFILE_INFO.informFormat.getId();
      long notifyId = rs.getLong(pos++); if (rs.wasNull()) notifyId = DEFAULT_PROFILE_INFO.notifyFormat.getId();
      int eventMask = rs.getInt(pos++);  if (rs.wasNull()) eventMask = ProfileInfo.MASK_ALL;
      info.informFormat = getFormatType(informId, true);
      info.notifyFormat = getFormatType(notifyId, false);
      info.eventMask = eventMask;
      return info;
    }
    catch (Exception exc) {
      logger.error("Query to DB failed", exc);
      throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
    }
    finally {
      try { if (rs != null) rs.close(); }
      catch (Throwable th) { logger.error(th); }
      try { if (stmt != null) stmt.close(); }
      catch (Throwable th) { logger.error(th); }
      try { if (connection != null) connection.close(); }
      catch (Throwable th) { logger.error(th); }
    }
  }

  private final int bindProfileInfo(PreparedStatement stmt, ProfileInfo info, int pos)
    throws SQLException
  {
    stmt.setString(pos++, info.inform ? "Y":"N");
    stmt.setString(pos++, info.notify ? "Y":"N");
    stmt.setLong  (pos++, info.informFormat.getId());
    stmt.setLong  (pos++, info.notifyFormat.getId());
    stmt.setInt   (pos++, info.eventMask);
    return pos;
  }
  public void setProfileInfo(String abonent, ProfileInfo info) throws ProfileManagerException
  {
    Connection connection  = null;
    PreparedStatement stmt = null;
    try
    {
      connection = ConnectionPool.getConnection();
      stmt = connection.prepareStatement(SET_PROFILE_SQL);
      int pos = bindProfileInfo(stmt, info, 1);
      stmt.setString(pos++, abonent);

      if (stmt.executeUpdate() <= 0) {
        stmt.close(); stmt = null; pos = 1;
        stmt = connection.prepareStatement(INS_PROFILE_SQL);
        stmt.setString(pos++, abonent);
        bindProfileInfo(stmt, info, pos);
        if (stmt.executeUpdate() <= 0)
          throw new Exception("Failed to insert new ProfileInfo record");
      }
      connection.commit();
    }
    catch (Exception exc) {
      try { if (connection != null) connection.rollback(); }
      catch (Throwable th) { logger.error("", th); }
      logger.error("Update/Insert to DB failed", exc);
      throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
    }
    finally {
      try { if (stmt != null) stmt.close(); }
      catch (Throwable th) { logger.error("", th); }
      try { if (connection != null) connection.close(); }
      catch (Throwable th) { logger.error("", th); }
    }
  }

  public void delProfileInfo(String abonent) throws ProfileManagerException
  {
    Connection connection  = null;
    PreparedStatement stmt = null;
    try
    {
      connection = ConnectionPool.getConnection();
      stmt = connection.prepareStatement(DEL_PROFILE_SQL);
      stmt.setString(1, abonent);
      stmt.executeUpdate();
      connection.commit();
    }
    catch (Exception exc) {
      try { if (connection != null) connection.rollback(); }
      catch (Throwable th) { logger.error("", th); }
      logger.error("Delete form DB failed", exc);
      throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
    }
    finally {
      try { if (stmt != null) stmt.close(); }
      catch (Throwable th) { logger.error("", th); }
      try { if (connection != null) connection.close(); }
      catch (Throwable th) { logger.error("", th); }
    }
  }

  public Iterator getFormatAlts(boolean inform) {
    return ((inform) ? informFormats:notifyFormats).values().iterator();
  }
}
