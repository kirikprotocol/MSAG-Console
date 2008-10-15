package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.ScenarioInitializationException;
//import ru.sibinco.smpp.appgw.util.ConnectionPool;
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
  private String hostProfStor = new String();
  private int portProfStor = 0;

  private final static String DEFAULT_INFORM = "defaultInform";
  private final static String DEFAULT_NOTIFY = "defaultNotify";
  private final static String DEFAULT_WANTNOTIFYME = "defaultWantNotifyMe";
  private final static String DEFAULT_REASONS_MASK = "defaultReasonsMask";
  private final static String DEFAULT_INFORM_ID = "defaultInformId";
  private final static String DEFAULT_NOTIFY_ID = "defaultNotifyId";
  private final static String INFORM_TEMPLATE_PREFIX = "informTemplate.";
  private final static String NOTIFY_TEMPLATE_PREFIX = "notifyTemplate.";
  private final static String HOST_PROF_STOR = "host";
  private final static String PORT_PROF_STOR = "port";

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

    strBool = properties.getProperty(DEFAULT_WANTNOTIFYME);
    if (strBool == null || strBool.trim().length() <= 0) {
      DEFAULT_PROFILE_INFO.wantNotifyMe = false;
      logger.warn("Parameter '"+DEFAULT_WANTNOTIFYME+"' missed. Default flag is off");
    }
    else DEFAULT_PROFILE_INFO.wantNotifyMe = strBool.trim().equalsIgnoreCase("true");

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

    InputStream isProfStor = this.getClass().getClassLoader().getResourceAsStream(Constants.MCI_PROF_STORAGE_FILE);
    if (isProfStor == null)
      throw new ScenarioInitializationException("Failed to locate profile storage properties file");

    Properties ProfStorProp = new Properties();
    try {
      ProfStorProp.load(isProfStor);
    } catch(IOException e) {
      throw new ScenarioInitializationException("Failed to load Profiles Storage properties", e);
    } finally {
      try { isProfStor.close(); } catch (Throwable th) {}
    }

    String host = ProfStorProp.getProperty(HOST_PROF_STOR);
    if (host == null || host.trim().length() <= 0) {
      logger.error("Parameter '"+HOST_PROF_STOR+"' missed. You must specify it.");
      throw new ScenarioInitializationException("Failed to load Profiles Storage property '" + HOST_PROF_STOR + "'");
    }
    hostProfStor = host;
    try {
      portProfStor = Integer.parseInt(ProfStorProp.getProperty(PORT_PROF_STOR));
    } catch (Exception e) {
        logger.error("Parameter '"+PORT_PROF_STOR+"' missed. You must specify it.");
        throw new ScenarioInitializationException("Failed to load Profiles Storage property '" + PORT_PROF_STOR + "'", e);
    }
  }

  private FormatType getFormatType(long id, boolean inform)
  {
    Long key = new Long(id);
    FormatType formatType = (FormatType)((inform ? informFormats:notifyFormats).get(key));
    if (formatType != null) return formatType;
    return (inform ? DEFAULT_PROFILE_INFO.informFormat : DEFAULT_PROFILE_INFO.notifyFormat);
  }

	private byte[] prepareStatement(String abnt, ProfileInfo info, boolean set)
	{
		byte[] stmt = new byte[41];
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
			stmt[38] = (info.isNotify() ? (byte)1:(byte)0);
			stmt[39] = (byte)info.getInformFormat().getId();
			stmt[40] = (byte)info.getNotifyFormat().getId();
		}
		return stmt;
	}

	private byte[] executeQuery(byte[] stmt) throws ProfileManagerException
	{
		Socket socket = null;
		InputStream input = null;
		OutputStream output = null;
		byte[] anw = new byte[41];
		int ret=0, len=0;
		try 
		{
			socket = new Socket(hostProfStor, portProfStor);
			socket.setSoTimeout(15000);
			input = socket.getInputStream();
			output = socket.getOutputStream();
			output.write(stmt, 0, 41);
			while(len < 41)
			{
				ret = input.read(anw, len, 41-len);
				len += ret;
			}
			if( anw[2] == 1)
			{
				logger.warn("ProfileInfo: Query to ProfileStorage - answer status STATUS_ERRROR");
				throw new ProfileManagerException(ProfileManagerException.DB_ERROR);
			}
            else if( anw[2] == 2)
			{
				logger.warn("ProfileInfo: Query to ProfileStorage - answer status STATUS_BADMSG");
				throw new ProfileManagerException(ProfileManagerException.DB_ERROR);
			}
			else if( anw[2] == 3)
			{
				logger.info("ProfileInfo: Query to ProfileStorage - answer status STATUS_NO_ABNT");
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
	}

  public ProfileInfo getProfileInfo(String abonent) throws ProfileManagerException
  {
	  try {
      ProfileInfo i = new ProfileInfo();
      byte[] stmt = prepareStatement(abonent, i, false);
      byte[] anw = executeQuery(stmt);

      ProfileInfo info = new ProfileInfo(getFormatType(anw[39], true), getFormatType(anw[40], false), anw[36]!=0, anw[37]!=0, anw[38]!=0, anw[35]);
      return info;
    } catch (Exception exc) {
      logger.error("get profile info failed", exc);
      throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
    }
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
  }

  public void delProfileInfo(String abonent) throws ProfileManagerException
  {
  }

  public Iterator getFormatAlts(boolean inform) {
    return ((inform) ? informFormats:notifyFormats).values().iterator();
  }
}
