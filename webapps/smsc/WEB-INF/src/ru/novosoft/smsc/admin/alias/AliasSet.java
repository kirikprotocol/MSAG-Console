/*
 * Author: igork
 * Date: 05.06.2002
 * Time: 16:52:19
 */
package ru.novosoft.smsc.admin.alias;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AliasDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.AliasQuery;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;


public class AliasSet
{
	private Set aliases = new HashSet();
	private AliasDataSource dataSource = new AliasDataSource();

	public AliasSet(Element aliasesElem)
	{
		NodeList aliasNodes = aliasesElem.getElementsByTagName("record");
		for (int i = 0; i < aliasNodes.getLength(); i++)
		{
			Element aliasElem = (Element) aliasNodes.item(i);
			add(new Alias(new Mask(aliasElem.getAttribute("addr")),
							  new Mask(aliasElem.getAttribute("alias")),
							  aliasElem.getAttribute("hide").equalsIgnoreCase("true"))
			);
		}
	}

	public PrintWriter store(PrintWriter out)
	{
		for (Iterator i = iterator(); i.hasNext();)
		{
			Alias a = (Alias) i.next();
			out.println("  <record addr=\"" + StringEncoderDecoder.encode(a.getAddress().getMask())
							+ "\" alias=\"" + StringEncoderDecoder.encode(a.getAlias().getMask())
							+ "\" hide=\"" + (a.isHide() ? "true" : "false")
							+ "\"/>");
		}
		return out;
	}

	public boolean add(Alias new_alias)
	{
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
}
