/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:03:51
 */
package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.admin.AdminException;

import java.io.PrintWriter;
import java.util.Collection;


public class Subject
{
	private String name = null;
	private SME defaultSme = null;
	private MaskList masks = null;

	public Subject(String name, String[] masksStrings, SME defaultSME) throws AdminException
	{
		if (name == null)
			throw new NullPointerException("Name is null");
		if (masksStrings == null)
			throw new NullPointerException("Masks is null");
		if (defaultSME == null)
			throw new NullPointerException("DefaultSME is null");

		this.name = name;
		masks = new MaskList(masksStrings);
		if (masks.size() == 0)
			throw new AdminException("Masks is empty");
		this.defaultSme = defaultSME;
	}

	public Subject(String name, Collection masksStrings, SME defaultSME)
	{
		if (name == null)
			throw new NullPointerException("Name is null");
		if (masksStrings == null)
			throw new NullPointerException("Masks is null");
		if (defaultSME == null)
			throw new NullPointerException("DefaultSME is null");

		this.name = name;
		masks = new MaskList(masksStrings);
		if (masks.size() == 0)
			throw new NullPointerException("Masks is empty");
		this.defaultSme = defaultSME;
	}

	public String getName()
	{
		return name;
	}

	public MaskList getMasks()
	{
		return masks;
	}

	public SME getDefaultSme()
	{
		return defaultSme;
	}

	public void setDefaultSme(SME defaultSme)
	{
		this.defaultSme = defaultSme;
	}

	public void setMasks(MaskList masks)
	{
		this.masks = masks;
	}

	public PrintWriter store(PrintWriter out)
	{
		out.println("  <subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\" defSme=\"" + StringEncoderDecoder.encode(getDefaultSme().getId()) + "\">");
		getMasks().store(out);
		out.println("  </subject_def>");
		return out;
	}
}
