/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:53
 */
package ru.novosoft.smsc.jsp.util.tables.impl.profile;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

import java.util.Vector;

public class ProfileQuery extends AbstractQueryImpl {
	public static final byte SHOW_ALL = 0;
	public static final byte SHOW_MASKS = 1;
	public static final byte SHOW_ADDRESSES = 2;

	private byte show;

	public ProfileQuery(int expectedResultsQuantity, ProfileFilter filter, Vector sortOrder, int startPosition, byte show)
	{
		super(expectedResultsQuantity, filter, sortOrder, startPosition);
		this.show = show;
	}

	public byte getShow()
	{
		return show;
	}

	public void setShow(byte show)
	{
		this.show = show;
	}
}
