/*
 * Author: igork
 * Date: 05.06.2002
 * Time: 16:52:19
 */
package ru.novosoft.smsc.admin.alias;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasQuery;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;


public class AliasSet
{
	private Set aliases = new HashSet();
	private AliasDataSource dataSource = new AliasDataSource();
	private Category logger = Category.getInstance(this.getClass());

	public AliasSet()
	{
	}

	public AliasSet(Element aliasesElem)
	{
		NodeList aliasNodes = aliasesElem.getElementsByTagName("record");
		for (int i = 0; i < aliasNodes.getLength(); i++)
		{
			Element aliasElem = (Element) aliasNodes.item(i);
			try
			{
				add(new Alias(new Mask(aliasElem.getAttribute("addr")), new Mask(aliasElem.getAttribute("alias")), aliasElem.getAttribute("hide").equalsIgnoreCase("true")));
			}
			catch (AdminException e)
			{
				logger.error("Couldn't load alias \"" + aliasElem.getAttribute("alias") + "\"-->\"" + aliasElem.getAttribute("addr") + "\", skipped", e);
			}
		}
	}

	public PrintWriter store(PrintWriter out)
	{
		for (Iterator i = iterator(); i.hasNext();)
		{
			Alias a = (Alias) i.next();
			out.println("  <record addr=\"" + StringEncoderDecoder.encode(a.getAddress().getMask()) + "\" alias=\"" + StringEncoderDecoder.encode(a.getAlias().getMask()) + "\" hide=\"" + (a.isHide() ? "true" : "false") + "\"/>");
		}
		return out;
	}

	public boolean add(Alias new_alias)
	{
		if (aliases.contains(new_alias))
			return false;

		if (new_alias.isHide())
			for (Iterator i = aliases.iterator(); i.hasNext();)
			{
				Alias alias = (Alias) i.next();
				if (alias.isHide() && alias.getAddress().equals(new_alias.getAddress()))
					return false;
			}

		dataSource.add(new_alias);
		return aliases.add(new_alias);
	}

	public Iterator iterator()
	{
		return aliases.iterator();
	}

	public boolean remove(Alias a)
	{
		dataSource.remove(a);
		return aliases.remove(a);
	}

	public boolean remove(String alias)
	{
		try
		{
			Alias a = new Alias(new Mask(alias), new Mask(alias), false);
			dataSource.remove(a);
			return aliases.remove(a);
		}
		catch (AdminException e)
		{
			logger.error("Couldn't remove alias \"" + alias + '"', e);
			return false;
		}
	}

	public QueryResultSet query(AliasQuery query)
	{
		dataSource.clear();
		for (Iterator i = aliases.iterator(); i.hasNext();)
		{
			Alias alias = (Alias) i.next();
			dataSource.add(alias);
		}
		return dataSource.query(query);
	}

	public boolean contains(Alias a)
	{
		return aliases.contains(a);
	}

	public Alias get(String aliasString)
	{
		for (Iterator i = aliases.iterator(); i.hasNext();)
		{
			Alias alias = (Alias) i.next();
			if (alias.getAlias().getMask().equals(aliasString))
				return alias;
		}
		return null;
	}

	public int size()
	{
		return aliases.size();
	}
}
