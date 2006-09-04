package ru.novosoft.smsc.mcisme.backend;

public class SchedItem
{
	public String schedTime = new String();
	public String address = new String();
	public String eventsCount = new String();
	public String lastError = new String();

	public SchedItem()
	{}
	public SchedItem(SchedItem item)
	{
		schedTime = item.schedTime;
		address = item.address;
		eventsCount = item.eventsCount;
		lastError = item.lastError;
	}
	public void reset()
	{
		schedTime = null;
		address = null;
		eventsCount = null;
		lastError = null;
	}
}
