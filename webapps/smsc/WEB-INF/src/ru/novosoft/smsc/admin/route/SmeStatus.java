package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;

import java.util.StringTokenizer;
import java.util.NoSuchElementException;

import org.apache.log4j.Category;

/**
 * Created by igork
 * Date: 06.05.2003
 * Time: 16:58:10
 */
public class SmeStatus
{
	private Category logger = Category.getInstance(Class.class);

	private String id;
	private boolean internal;
	private boolean connected;
	private byte bindMode;
	private String inAddress;
	private String outAddress;

	public SmeStatus(String id, boolean internal, boolean connected, byte bindMode, String inAddress, String outAddress)
	{
		this.id = id;
		this.internal = internal;
		this.connected = connected;
		this.bindMode = bindMode;
		this.inAddress = inAddress;
		this.outAddress = outAddress;
	}

	public SmeStatus(String initString) throws AdminException
	{
		//logger.debug("init string: " + initString);
		StringTokenizer tokenizer = new StringTokenizer(initString, ",", false);
		try
		{
			id = tokenizer.nextToken();
			String connectStr = tokenizer.nextToken();
			if (connectStr.equalsIgnoreCase("internal"))
			{
				internal = true;
				connected = true;
				bindMode = SME.MODE_TRX;
				inAddress = outAddress = "";
			}
			else
			{
				internal = false;
				if (connectStr.equalsIgnoreCase("tx"))
				{
					connected = true;
					bindMode = SME.MODE_TX;
				}
				else if (connectStr.equalsIgnoreCase("rx"))
				{
					connected = true;
					bindMode = SME.MODE_RX;
				}
				else if (connectStr.equalsIgnoreCase("trx"))
				{
					connected = true;
					bindMode = SME.MODE_TRX;
				}
				else if (connectStr.equalsIgnoreCase("disconnected"))
				{
					connected = false;
					bindMode = SME.MODE_UNKNOWN;
				}
				else
				{
					connected = true;
					bindMode = SME.MODE_UNKNOWN;
				}

				if (connected)
				{
					inAddress = tokenizer.nextToken();
					outAddress = tokenizer.nextToken();
				}
				else
				{
					inAddress = outAddress = "";
				}
			}
		}
		catch (NoSuchElementException e)
		{
			logger.debug("Init string misformatted", e);
			throw new AdminException("Init string misformatted, nested: " + e.getMessage());
		}
		//logger.debug("result: "+id+"[int="+internal+",conn="+connected+",mode="+bindMode+",("+inAddress+"|"+outAddress+")");
	}

	public String getId()
	{
		return id;
	}

	public boolean isInternal()
	{
		return internal;
	}

	public boolean isConnected()
	{
		return connected;
	}

	public byte getBindMode()
	{
		return bindMode;
	}

	public String getInAddress()
	{
		return inAddress;
	}

	public String getOutAddress()
	{
		return outAddress;
	}
}
