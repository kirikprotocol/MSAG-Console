/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:53
 */
package ru.novosoft.smsc.jsp.util.tables.impl.profile;


public class ProfileQuery {
	public static final byte SHOW_ALL = 0;
	public static final byte SHOW_MASKS = 1;
	public static final byte SHOW_ADDRESSES = 2;

	private int expectedResultsQuantity;
	private String filter;
	private String sortOrder;
	private int startPosition;
	private byte show;

	public ProfileQuery(int expectedResultsQuantity, String filter, String sortOrder, int startPosition, byte show)
	{
		this.expectedResultsQuantity = expectedResultsQuantity;
		this.filter = filter;
		this.sortOrder = sortOrder;
		this.startPosition = startPosition;
		this.show = show;
	}

	public int getExpectedResultsQuantity()
	{
		return expectedResultsQuantity;
	}

	public String getFilter()
	{
		return filter;
	}

	public String getSortOrder()
	{
		return sortOrder;
	}

	public int getStartPosition()
	{
		return startPosition;
	}

	public byte getShow()
	{
		return show;
	}
}
