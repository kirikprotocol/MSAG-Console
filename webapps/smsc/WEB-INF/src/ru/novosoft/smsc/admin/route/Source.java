/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 14:03:03
 */
package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;


public class Source
{
	Subject subj = null;
	Mask mask = null;

	public Source(Subject s)
	{
		if (s == null)
			throw new NullPointerException("Subject is null");

		subj = s;
	}

	public Source(Element srcElem, SubjectList subjects)
	{
		NodeList list = srcElem.getElementsByTagName("subject");
		if (list.getLength() > 0)
		{
			Element subjElem = (Element) list.item(0);
			subj = subjects.get(subjElem.getAttribute("id"));
			if (subj == null)
				throw new NullPointerException("Subject is unknown");
		}
		else
		{
			list = srcElem.getElementsByTagName("mask");
			Element maskElem = (Element) list.item(0);
			mask = new Mask(maskElem.getAttribute("value"));
		}
	}

	public Source(Mask m)
	{
		if (m == null)
			throw new NullPointerException("Mask is null");

		mask = m;
	}

	public boolean isSubject()
	{
		return subj != null;
	}

	public String getName()
	{
		if (isSubject())
			return subj.getName();
		else
			return mask.getMask();
	}

	public MaskList getMasks()
	{
		if (isSubject())
			return subj.getMasks();
		else
			return new MaskList(mask);
	}

	public PrintWriter store(PrintWriter out)
	{
		out.println("    <source>");
		if (isSubject())
			out.println("      <subject id=\"" + StringEncoderDecoder.encode(subj.getName()) + "\"/>");
		else
			out.println("      <mask value=\"" + StringEncoderDecoder.encode(mask.getMask()) + "\"/>");
		out.println("    </source>");
		return out;
	}

	public String getDefaultSmeId()
	{
		if (subj != null)
			return subj.getDefaultSme().getId();
		else
			return null;
	}
}
