/*
 * Author: igork
 * Date: 28.05.2002
 * Time: 17:32:54
 */
package ru.novosoft.smsc.admin.route;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.*;


public class SMEList
{
	private Category logger = Category.getInstance(this.getClass());
	private Map smes = null;

	public SMEList()
	{
		smes = new HashMap();
	}

	public SMEList(Element smeListElement)
	{
		this();
		NodeList list = smeListElement.getElementsByTagName("smerecord");
		for (int i = 0; i < list.getLength(); i++)
		{
			try
			{
				add(new SME((Element) list.item(i)));
			}
			catch (AdminException e)
			{
				logger.error("Error inc SME config", e);
			}
		}
	}

	public int size()
	{
		return smes.size();
	}

	public Iterator iterator()
	{
		return smes.values().iterator();
	}

	public Set getNames()
	{
		return smes.keySet();
	}

	public SME add(SME sme) throws AdminException
	{
		if (smes.containsKey(sme.getId()))
			throw new AdminException("Sme \"" + sme.getId() + "\" already contained in list");
		smes.put(sme.getId(), sme);
		return sme;
	}

	public SME get(String smeId) throws AdminException
	{
		require(smeId);
		return (SME) smes.get(smeId);
	}

	public SME remove(String smeId) throws AdminException
	{
		require(smeId);
		return (SME) smes.remove(smeId);
	}

	public PrintWriter store(PrintWriter out)
	{
		List values = new LinkedList(smes.values());
		Collections.sort(values, new Comparator(){
			public int compare(Object o1, Object o2)
			{
				SME s1 = (SME) o1;
				SME s2 = (SME) o2;
				return s1.getId().compareTo(s2.getId());
			}
		});
		for (Iterator i = values.iterator(); i.hasNext();)
			((SME) i.next()).store(out);
		return out;
	}

	public boolean contains(String smeId)
	{
		return smes.containsKey(smeId);
	}

	private void require(String smeId) throws AdminException
	{
		if (!smes.containsKey(smeId))
			throw new AdminException("SME \"" + smeId + "\" not found");
	}

	public SME update(SME newSme) throws AdminException
	{
		SME oldSme = get(newSme.getId());
		oldSme.update(newSme);
		return oldSme;
	}

}
