package ru.novosoft.smsc.mcisme.backend;

import java.util.Iterator;
import java.util.Properties;
import java.util.HashMap;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.ResultSet;
import java.io.*;
import java.net.*;
import ru.novosoft.smsc.mcisme.backend.ProfileInfo;
import ru.novosoft.smsc.mcisme.backend.ProfilePacket;
import org.apache.log4j.Logger;

public class ProfStorageAccessor
{
  static Logger logger = Logger.getLogger(ProfStorageAccessor.class);

  private String host;
	private int port;
	private ProfilePacket packet=null;

	public ProfStorageAccessor(String _host, int _port)
	{
		host=_host; port=_port;
		packet = new ProfilePacket();
	}

	public int getProfile(String Address, ProfileInfo profile)
	{
//		profile.inform = true;
//		profile.notify = true;
//	
//		profile.informSelectedTemplate = 2;
//		profile.notifySelectedTemplate = 3;
//	
//		profile.busy = true;
//		profile.noReplay = true;
//		profile.absent = true;
//		profile.detach = true;
//		profile.uncond = true;
//		profile.other = true;
//		return 0;

		packet.preparePacket(Address, profile, false);
		if(!executeQuery())	return 1;
		packet.extractProfileInfo(profile);
		return 0;
	}

	public int setProfile(String Address, ProfileInfo profile)
	{
		packet.preparePacket(Address, profile, true);
		if(!executeQuery())	return 1;
		return 0;
	}

	private boolean executeQuery()
	{
		Socket socket = null;
		InputStream input = null;
		OutputStream output = null;

		byte[] anw = new byte[ProfilePacket.PACKET_LEN];
		int ret=0, len=0;
		try 
		{
			socket = new Socket(host, port);
			socket.setSoTimeout(15000);
			input = socket.getInputStream();
			output = socket.getOutputStream();
			output.write(packet.getPacket(), 0, packet.getPacketLen());
			while(len < ProfilePacket.PACKET_LEN)
			{
				ret = input.read(anw, len, ProfilePacket.PACKET_LEN-len);
				len += ret;
			}
			input.close();
			output.close();
			socket.close();
			packet.setPacket(anw);
			return true;
		} 
		catch(UnknownHostException exc) 
		{
      logger.error("Could not get MCI profile", exc);
      return false;
		} 
		catch(IOException exc) 
		{
      logger.error("Could not get MCI profile", exc);
			return false;		}
		//catch(SocketTimeoutException exc) 
		//{
		//	logger.warn("ProfileInfo: Query to ProfileStorage");
		//	throw new ProfileManagerException(exc, ProfileManagerException.DB_ERROR);
		//}
	}

}
