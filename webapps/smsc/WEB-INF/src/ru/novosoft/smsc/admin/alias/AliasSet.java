/*
 * Author: igork
 * Date: 05.06.2002
 * Time: 16:52:19
 */
package ru.novosoft.smsc.admin.alias;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.Set;
import java.util.HashSet;
import java.util.Iterator;
import java.io.OutputStream;
import java.io.PrintWriter;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.util.StringEncoderDecoder;


public class AliasSet
{
  private Set aliases = new HashSet();

  public AliasSet(Element aliasesElem)
  {
    NodeList aliasNodes = aliasesElem.getElementsByTagName("record");
    for (int i = 0; i< aliasNodes.getLength(); i++)
    {
      Element aliasElem = (Element) aliasNodes.item(i);
      add(new Alias(new Mask(aliasElem.getAttribute("addr")), new Mask(aliasElem.getAttribute("alias"))));
    }
  }

  public PrintWriter store(PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext(); )
    {
      Alias a = (Alias) i.next();
      out.println("  <record addr=\"" + StringEncoderDecoder.encode(a.getAddress().getMask())
                  + "\" alias=\"" + StringEncoderDecoder.encode(a.getAlias().getMask())
                  + "\"/>");
    }
    return out;
  }

  public boolean add(Alias new_alias)
  {
    return aliases.add(new_alias);
  }

  public Iterator iterator()
  {
    return aliases.iterator();
  }

  public boolean remove(Alias a)
  {
    return aliases.remove(a);
  }
}
