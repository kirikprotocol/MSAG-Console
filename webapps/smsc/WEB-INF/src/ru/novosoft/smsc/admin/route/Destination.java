/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:45:30
 */
package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Element;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;


public class Destination extends Source
{
	private SME sme = null;

	public Destination(Subject subj)
			throws AdminException
	{
		this(subj, subj.getDefaultSme());
	}

	public Destination(Subject subj, SME sme)
			throws AdminException
	{
		super(subj);

		if ((this.sme = sme) == null)
			throw new AdminException("SME is null");
	}

	public Destination(Mask mask, SME sme)
			throws AdminException
	{
		super(mask);
		if ((this.sme = sme) == null)
			throw new AdminException("SME is null");
	}

	public Destination(Element dstElem, SubjectList subjects, SMEList smes)
			throws AdminException
	{
		super(dstElem, subjects);
		sme = smes.get(dstElem.getAttribute("sme"));
		if (sme == null)
			throw new AdminException("Unknown SME \"" + dstElem.getAttribute("sme") + '"');
	}

	public SME getSme()
	{
		return sme;
	}

	public void setSme(SME sme)
	{
		this.sme = sme;
	}

	public PrintWriter store(PrintWriter out)
	{
		out.println("    <destination sme=\"" + StringEncoderDecoder.encode(getSme().getId()) + "\">");
		if (isSubject())
			out.println("      <subject id=\"" + StringEncoderDecoder.encode(subj.getName()) + "\"/>");
		else
			out.println("      <mask value=\"" + StringEncoderDecoder.encode(mask.getNormalizedMask()) + "\"/>");
		out.println("    </destination>");
		return out;
	}
}
