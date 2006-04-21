package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
import ru.sibinco.smpp.appgw.util.ConnectionPool;
import ru.sibinco.mci.Constants;

import java.util.Iterator;
import java.util.Properties;
import java.util.HashMap;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.ResultSet;
import java.io.*;
import java.net.*;

import org.apache.log4j.Category;

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

  private final static String DEFAULT_INFORM = "defaultInform";
  private final static String DEFAULT_NOTIFY = "defaultNotify";
  private final static String DEFAULT_REASONS_MASK = "defaultReasonsMask";
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

    try {
      DEFAULT_PROFILE_INFO.eventMask = Integer.parseInt(properties.getProperty(DEFAULT_REASONS_MASK));
    } catch (Exception e) {
      DEFAULT_PROFILE_INFO.eventMask = ProfileInfo.MASK_ALL;
      logger.warn("Parameter '"+DEFAULT_REASONS_MASK+"' missed. All masks on by default");
    }

    String strBool = properties.getProperty(DEFAULT_INFORM);
    if (strBool == null || strBool.trim().length() <= 0) {
      DEFAULT_PROFILE_INFO.inform = true;
      logger.warn("Parameter '"+DEFAULT_INFORM+"' missed. Default flag is on");
    }
    else DEFAULT_PROFILE_INFO.inform = strBool.trim().equalsIgnoreCase("true");
    
    strBool = properties.getProperty(DEFAULT_NOTIFY);
    if (strBool == null || strBool.trim().length() <= 0) {
      DEFAULT_PROFILE_INFO.notify = false;
      logger.warn("Parameter '"+DEFAULT_NOTIFY+"' missed. Default flag is off");
    }
    else DEFAULT_PROFILE_INFO.notify = strBool.trim().equalsIgnoreCase("true");

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

	private byte[] prepareStatement(String abnt, ProfileInfo info, boolean set)
	{
		byte[] stmt = new byte[40];
		int i;

		stmt[0] = 1;
		stmt[1]=0;	stmt[2]=0;
		for(i = 0; i < abnt.length(); i++)
			stmt[i+3] = (byte)abnt.charAt(i);
		if(set)
		{
			stmt[0] = 2;
			stmt[35] = (byte)info.getEventMask();
			stmt[36] = (info.isInform() ? (byte)1:(byte)0);
			stmt[37] = (info.isNotify() ? (byte)1:(byte)0);
			stmt[38] = (byte)info.getInformFormat().getId();
			stmt[39] = (byte)info.getNotifyFormat().getId();
		}
		return stmt;
	}

	private byte[] executeQuery(byte[] stmt) throws ProfileManagerException
	{
		ProfileInfo info = new ProfileInfo(DEFAULT_PROFILE_INFO);
		Socket socket = null;
		InputStream input = null;
		OutputStream output = null;
		byte[] anw = new byte[40];
		int ret=0, len=0;
		try 
		{
			socket = new Socket("0.0.0.0", 39999);
			socket.setSoTimeout(15000);
			input = socket.getInputStream();
			output = socket.getOutputStream();
//			logger.error("Query to DB failed");
			output.write(stmt, 0, 40);
			while(len < 40)
			{
				ret = input.read(anw, len, 40-len);
				len += ret;
			}
			if(anw[2] != 0)
			{
				logger.warn("ProfileInfo: Query to ProfileStorage - answer status != 0");
				throw new ProfileManagerException(ProfileManagerException.DB_ERROR);
			}
			input.close();
			output.close();
			socket.close();
			return anw;

		} 
		catch(UnknownHostException exc) 
		{
			logger.warn("ProfileInfo: Query to ProfileStorage");
			throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
		} 
		catch(IOException exc) 
		{
			logger.warn("ProfileInfo: Query to ProfileStorage");
			throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
		}
//		catch(SocketTimeoutException exc) 
//		{
//			logger.warn("ProfileInfo: Query to ProfileStorage");
//			throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
//		}
	}

  public ProfileInfo getProfileInfo(String abonent) throws ProfileManagerException
  {
//    Connection connection  = null;
//    PreparedStatement stmt = null;
//    ResultSet rs = null;
	
	try
    {
//      connection = ConnectionPool.getConnection();
//      stmt = connection.prepareStatement(GET_PROFILE_SQL);
//      stmt.setString(1, abonent);
//      rs = stmt.executeQuery();
//
//      if (rs == null || !rs.next())
//        return new ProfileInfo(DEFAULT_PROFILE_INFO);
		ProfileInfo i = new ProfileInfo();
		byte[] stmt = prepareStatement(abonent, i, false);
		byte[] anw = executeQuery(stmt);

		ProfileInfo info = new ProfileInfo(getFormatType(anw[38], true), getFormatType(anw[39], false), (anw[36]==0? false: true), (anw[37]==0? false: true), anw[35]);
//		info.eventMask = anw[35];
//		info.inform = (anw[36]==0? false: true);
//		info.notify = (anw[37]==0? false: true);
//		info.informFormat = getFormatType(anw[38], true);
//		info.notifyFormat = getFormatType(anw[39], false);
//		ProfileInfo info = new ProfileInfo(getFormatType(1, true), getFormatType(1, false), true, true, 111);

		return info;
	
//		int pos = 1;
//		String result = rs.getString(pos++);
//		if (rs.wasNull()) info.inform = DEFAULT_PROFILE_INFO.inform;
//		else info.inform = !(result == null || result.length() <= 0 || result.trim().equalsIgnoreCase("N"));
//		result = rs.getString(pos++);
//		if (rs.wasNull()) info.notify = DEFAULT_PROFILE_INFO.notify;
//		else info.notify = !(result == null || result.length() <= 0 || result.trim().equalsIgnoreCase("N"));
//		long informId  = rs.getLong(pos++); if (rs.wasNull()) informId = DEFAULT_PROFILE_INFO.informFormat.getId();
//		long notifyId  = rs.getLong(pos++); if (rs.wasNull()) notifyId = DEFAULT_PROFILE_INFO.notifyFormat.getId();
//		info.eventMask = rs.getInt (pos++); if (rs.wasNull()) info.eventMask = DEFAULT_PROFILE_INFO.eventMask;
//		info.informFormat = getFormatType(informId, true);
//		info.notifyFormat = getFormatType(notifyId, false);
//		return info;
    }
    catch (Exception exc) {
      logger.error("Query to DB failed", exc);
      throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
    }
//	catch (ProfileManagerException exc) 
//	{
//		logger.error("Query to DB failed", exc);
//		throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
//	}

	finally 
	{
//      try { if (rs != null) rs.close(); } catch (Throwable th) { logger.error(th); }
//      try { if (stmt != null) stmt.close(); } catch (Throwable th) { logger.error(th); }
//      try { if (connection != null) connection.close(); } catch (Throwable th) { logger.error(th); }
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
	  try
	  {
		  byte[] stmt = prepareStatement(abonent, info, true);
		  byte[] anw = executeQuery(stmt);
	  }
	  catch (Exception exc) 
	  {
		  logger.error("Query to DB failed", exc);
		  throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
	  }


//    Connection connection  = null;
//    PreparedStatement stmt = null;
//    try
//    {
//      connection = ConnectionPool.getConnection();
//      stmt = connection.prepareStatement(SET_PROFILE_SQL);
//      int pos = bindProfileInfo(stmt, info, 1);
//      stmt.setString(pos++, abonent);
//
//      if (stmt.executeUpdate() <= 0) {
//        stmt.close(); stmt = null; pos = 1;
//        stmt = connection.prepareStatement(INS_PROFILE_SQL);
//        stmt.setString(pos++, abonent);
//        bindProfileInfo(stmt, info, pos);
//        if (stmt.executeUpdate() <= 0)
//          throw new Exception("Failed to insert new ProfileInfo record");
//      }
//      connection.commit();
//    }
//    catch (Exception exc) {
//      try { if (connection != null) connection.rollback(); } catch (Throwable th) { logger.error("", th); }
//      logger.error("Update/Insert to DB failed", exc);
//      throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
//    }
//    finally {
//      try { if (stmt != null) stmt.close(); } catch (Throwable th) { logger.error("", th); }
//      try { if (connection != null) connection.close(); }  catch (Throwable th) { logger.error("", th); }
//    }
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
      try { if (connection != null) connection.rollback(); } catch (Throwable th) { logger.error("", th); }
      logger.error("Delete form DB failed", exc);
      throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
    }
    finally {
      try { if (stmt != null) stmt.close(); } catch (Throwable th) { logger.error("", th); }
      try { if (connection != null) connection.close(); } catch (Throwable th) { logger.error("", th); }
    }
  }

  public Iterator getFormatAlts(boolean inform) {
    return ((inform) ? informFormats:notifyFormats).values().iterator();
  }
}
