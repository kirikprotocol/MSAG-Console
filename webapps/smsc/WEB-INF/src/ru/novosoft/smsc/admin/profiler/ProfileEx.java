package ru.novosoft.smsc.admin.profiler;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

/**
 * Created by igork
 * Date: Jul 23, 2003
 * Time: 5:32:48 PM
 */
public class ProfileEx extends Profile {
	public static final byte MATCH_UNKNOWN = -1;
	public static final byte MATCH_DEFAULT = 0;
	public static final byte MATCH_EXACT = 1;
	public static final byte MATCH_MASK = 2;

	private byte matchType = MATCH_UNKNOWN;
	private Mask matchAddress = null;

	public ProfileEx(Mask mask, List profileProperties) throws AdminException
	{
		super(mask, profileProperties);
		setMatchType((String) profileProperties.get(5));
		if (matchType != MATCH_DEFAULT)
		setMatchAddress(new Mask((String) profileProperties.get(6)));
	}

	public byte getMatchType()
	{
		return matchType;
	}

	public void setMatchType(byte matchType)
	{
		this.matchType = matchType;
	}

	public void setMatchType(String matchType)
	{
		if (matchType == null || matchType.length() == 0)
			this.matchType = MATCH_UNKNOWN;
		else if (matchType.equals("default"))
			this.matchType = MATCH_DEFAULT;
		else if (matchType.equals("exact"))
			this.matchType = MATCH_EXACT;
		else if (matchType.equals("mask"))
			this.matchType = MATCH_MASK;
		else
			this.matchType = MATCH_UNKNOWN;
	}

	public static String getMatchTypeString(byte matchType)
	{
		switch (matchType) {
			case MATCH_DEFAULT:
				return "default";
			case MATCH_EXACT:
				return "exact";
			case MATCH_MASK:
				return "mask";
			default:
				return "unknown";
		}
	}

	public Mask getMatchAddress()
	{
		return matchAddress;
	}

	public void setMatchAddress(Mask matchAddress)
	{
		this.matchAddress = matchAddress;
	}
}
