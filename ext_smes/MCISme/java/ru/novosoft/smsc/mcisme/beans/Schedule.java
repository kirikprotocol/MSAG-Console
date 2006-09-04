package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.mcisme.backend.MCISme;
import ru.novosoft.smsc.mcisme.backend.SchedItem;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.io.*;

/**
 * User: miker
 * Date: 30.08.2006
 */
public class Schedule extends MCISmeBean
{
	private String mbQuery = null;
	private String mbRefresh = null;

	private MCISme mciSme;
	private SchedItem	item = new SchedItem();
	private String address = null;

//	private String schedTime = null;
//	private String eventsCount = null;
//	private String lastError = null;

	public int process(HttpServletRequest request)
	{
		int result = super.process(request);
		if (result != RESULT_OK)  return result;

		mciSme = getMCISmeContext().getMCISme();

		if(mbQuery != null)	return onQuery();
//		if(mbRefresh != null) return onRefresh();

		return result;
	}
	private int onQuery()
	{
		if(!checkAddress())
		{
			address = null;
			return error("mcisme.error.bad_address");
		}

		try 
		{
			if (mciSme.getInfo().isOnline()) item = mciSme.getSchedItem(address);
			else error("MCISme is offline");
				
		} 
		catch (AdminException e) 
		{
			error("AdminExeption handled.");;
			e.printStackTrace();
		}

		return RESULT_OK;
	}
	private int onRefresh()
	{
		return RESULT_OK;
	}
	private boolean checkAddress()
	{
		return true;
	}
	public String getMbQuery(){return mbQuery;}
	public void setMbQuery(String value){this.mbQuery = value;}
	public String getMbRefresh(){return mbRefresh;}
	public void setMbRefresh(String value){this.mbRefresh = value;}

	public String getAddress(){return address;}
	public void setAddress(String value){this.address = value;}

	public String getSchedTime(){return item.schedTime;}
	public String getEventsCount(){return item.eventsCount;}
	public String getLastError(){return item.lastError;}

	public List getAbonentsList()
	{
		ArrayList itemsList = new ArrayList();
		try 
		{
			if (mciSme.getInfo().isOnline()) itemsList = mciSme.getSchedItems();
			else error("MCISme is offline");
				
		} 
		catch (AdminException e) 
		{
			error("AdminExeption handled.");;
			e.printStackTrace();
		}
		return itemsList;
	}
}
