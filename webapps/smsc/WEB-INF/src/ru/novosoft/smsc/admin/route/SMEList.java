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

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;


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
        logger.error("Duplicate SME definition", e);
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

  public void add(SME sme)
          throws AdminException
  {
    if (smes.containsKey(sme.getId()))
      throw new AdminException("Sme \"" + sme.getId() + "\" already contained in list");
    smes.put(sme.getId(), sme);
  }

  public SME get(String smeId)
  {
    return (SME) smes.get(smeId);
  }

  public SME remove(String smeId)
  {
    return (SME) smes.remove(smeId);
  }

  public PrintWriter store(PrintWriter out)
  {
    for (Iterator i = smes.values().iterator(); i.hasNext();)
    {
      SME sme = (SME) i.next();
      out.println("  <smerecord type=\"" + (sme.getType() == SME.SMPP ? "smpp" : "ss7") + "\" uid=\"" + sme.getId() + "\">");

      out.println("    <param name=\"typeOfNumber\" value=\"" + sme.getTypeOfNumber() + "\"/>");
      out.println("    <param name=\"numberingPlan\" value=\"" + sme.getNumberingPlan() + "\"/>");
      out.println("    <param name=\"interfaceVersion\" value=\"" + sme.getInterfaceVersion() + "\"/>");
      out.println("    <param name=\"systemType\" value=\"" + sme.getSystemType() + "\"/>");
      out.println("    <param name=\"password\" value=\"" + sme.getPassword() + "\"/>");
      out.println("    <param name=\"addrRange\" value=\"" + sme.getAddrRange() + "\"/>");
      out.println("    <param name=\"smeN\" value=\"" + sme.getSmeN() + "\"/>");

      out.println("  </smerecord>");
    }
    return out;
  }
}
