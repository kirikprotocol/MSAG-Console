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

public class ProfilePacket
{
	private final static int MASK_NONE    = 0x00;
	private final static int MASK_ABSENT  = 0x01;
	private final static int MASK_BUSY    = 0x02;
	private final static int MASK_NOREPLY = 0x04;
	private final static int MASK_UNCOND  = 0x08;
	private final static int MASK_DETACH  = 0x10;
	private final static int MASK_ALL     = 0xFF;

	private final static byte IDX_MSG_ID		= 0;
	private final static byte IDX_SEQ_NUM		= 1;
	private final static byte IDX_STATUS		= 2;
	private final static byte IDX_ADDR			= 3;
	private final static byte IDX_EVENT_MASK	= 35;
	private final static byte IDX_INFORM		= 36;
	private final static byte IDX_NOTIFY		= 37;
	private final static byte IDX_INF_TEMPL_ID	= 38;
	private final static byte IDX_NOT_TEMPL_ID	= 39;
  private final static byte IDX_WANT_NOTIFY		= 40;

	private final static int PACKET_LEN			= 41;

	private final static int MSG_ID_PROFILE		= 0;
	private final static int MSG_ID_GETPROF		= 1;
	private final static int MSG_ID_SETPROF		= 2;

	private final static int STATUS_OK			= 0;
	private final static int STATUS_ERRROR		= 1;
	private final static int STATUS_BADMSG		= 2;
	private final static int STATUS_NO_ABNT		= 3;

	private byte[] packet = new byte[PACKET_LEN];
	private byte seqNum=0;

	public ProfilePacket()
	{
	}
	public ProfilePacket(String abnt, ProfileInfo info, boolean set)
	{
		preparePacket(abnt, info, set);
	}

	public byte[] getPacket()
	{
		return packet;
	}
	public int getPacketLen()
	{
		return PACKET_LEN;
	}
	public void setPacket(byte[] pack)
	{
		for(int i = 0; i < PACKET_LEN; i++)
			packet[i] = pack[i];
	}
	public void preparePacket(String abnt, ProfileInfo info, boolean set)
	{
		packet[IDX_MSG_ID]	= MSG_ID_GETPROF;
		packet[IDX_SEQ_NUM]	= seqNum++;
		packet[IDX_STATUS]	= STATUS_OK;

		for(int i = 0; i < abnt.length(); i++)
			packet[IDX_ADDR + i] = (byte)abnt.charAt(i);
		if(set)
		{
			byte eventMask = calcEventMask(info);

			packet[IDX_MSG_ID]			= MSG_ID_SETPROF;
			packet[IDX_EVENT_MASK]		= eventMask;
			packet[IDX_INFORM]			= (info.inform ? (byte)1:(byte)0);
			packet[IDX_NOTIFY]			= (info.notify ? (byte)1:(byte)0);
			packet[IDX_INF_TEMPL_ID]	= (byte)info.informSelectedTemplate;
			packet[IDX_NOT_TEMPL_ID]	= (byte)info.notifySelectedTemplate;
      packet[IDX_WANT_NOTIFY]			= (info.wantNotifyMe ? (byte)1:(byte)0);
		}
	}

	private byte calcEventMask(ProfileInfo info)
	{
		byte mask = MASK_NONE;

		if(info.busy) mask += MASK_BUSY;	
		if(info.noReplay) mask += MASK_NOREPLY;
		if(info.absent) mask += MASK_ABSENT;
		if(info.detach) mask += MASK_DETACH;
		if(info.uncond) mask += MASK_UNCOND;
//		if(info.other) mask += MASK_ALL;

		return mask;
	}

	public void extractProfileInfo(ProfileInfo info)
	{
		info.inform = (packet[IDX_INFORM]==1)?true:false;
		info.notify = (packet[IDX_NOTIFY]==1)?true:false;
    info.wantNotifyMe = (packet[IDX_WANT_NOTIFY]==1)?true:false;
		info.informSelectedTemplate = (int)packet[IDX_INF_TEMPL_ID];
		info.notifySelectedTemplate = (int)packet[IDX_NOT_TEMPL_ID];
		if(packet[IDX_STATUS] == STATUS_NO_ABNT)
			info.isAddressNotExists = true;
		else
			info.isAddressNotExists = false;
		info.busy		= ((packet[IDX_EVENT_MASK] & MASK_BUSY) == MASK_BUSY) ? true: false;
		info.noReplay	= ((packet[IDX_EVENT_MASK] & MASK_NOREPLY) == MASK_NOREPLY) ? true: false;
		info.absent		= ((packet[IDX_EVENT_MASK] & MASK_ABSENT) == MASK_ABSENT) ? true: false;
		info.detach		= ((packet[IDX_EVENT_MASK] & MASK_DETACH) == MASK_DETACH) ? true: false;
		info.uncond		= ((packet[IDX_EVENT_MASK] & MASK_UNCOND) == MASK_UNCOND) ? true: false;
	}
}

