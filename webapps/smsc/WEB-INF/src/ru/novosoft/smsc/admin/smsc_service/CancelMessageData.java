package ru.novosoft.smsc.admin.smsc_service;

/**
 * Created by igork
 * Date: Dec 23, 2002
 * Time: 7:22:37 PM
 */
public class CancelMessageData
{
	private String messageId = null;
	private String sourceAddress = null;
	private String destinationAddress = null;

	public CancelMessageData(String messageId, String sourceAddress, String destinationAddress)
	{
		this.messageId = messageId;
		this.sourceAddress = sourceAddress;
		this.destinationAddress = destinationAddress;
	}

	public String getMessageId()
	{
		return messageId;
	}

	public String getSourceAddress()
	{
		return sourceAddress;
	}

	public String getDestinationAddress()
	{
		return destinationAddress;
	}
}
