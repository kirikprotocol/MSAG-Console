/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:43:03
 */
package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Element;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.List;
import java.util.Set;


public class Route
{
  private String name = null;
  private SourceList src = null;
  private DestinationList dst = null;
  private boolean enabling = true;
  private boolean archiving = true;
  private boolean billing = false;

  public Route(String routeName, boolean isEnabling, SourceList sources, DestinationList destinations)
  {
    if (routeName == null)
      throw new NullPointerException("Route name is null");
    if (sources == null)
      throw new NullPointerException("Sources list is null");
    if (destinations == null)
      throw new NullPointerException("Destinations list is null");

    name = routeName;
    src = sources;
    dst = destinations;
    enabling = isEnabling;
  }

  public Route(String routeName)
  {
    if (routeName == null)
      throw new NullPointerException("Route name is null");

    name = routeName;
    src = new SourceList();
    dst = new DestinationList();
    enabling = false;
  }

  public Route(Element routeElem, SubjectList subjects, SMEList smes)
          throws AdminException
  {
    name = routeElem.getAttribute("id");
    src = new SourceList(routeElem, subjects);
    dst = new DestinationList(routeElem, subjects, smes);
    enabling = routeElem.getAttribute("enabling").equalsIgnoreCase("true");
    archiving = routeElem.getAttribute("archiving").equalsIgnoreCase("true");
    billing = routeElem.getAttribute("billing").equalsIgnoreCase("true");
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }

  public void addSource(Source newSrc)
  {
    if (newSrc == null)
      throw new NullPointerException("Source is null");

    src.add(newSrc);
  }

  public Source removeSource(String sourceName)
  {
    return src.remove(sourceName);
  }

  public Destination removeDestination(String destinationName)
  {
    return dst.remove(destinationName);
  }

  public void addDestination(Destination newDst)
  {
    if (newDst == null)
      throw new NullPointerException("Destination is null");

    dst.add(newDst);
  }

  public SourceList getSources()
  {
    return src;
  }

  public DestinationList getDestinations()
  {
    return dst;
  }

  public void updateSources(Set sourcesStrings, String masksString, SubjectList allSubjects)
  {
    SourceList source_selected = new SourceList();
    for (Iterator i = sourcesStrings.iterator(); i.hasNext(); )
    {
      String token = (String) i.next();
      source_selected.add(new Source(allSubjects.get(token)));
    }

    MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();)
    {
      Mask m = (Mask) i.next();
      source_selected.add(new Source(m));
    }

    src.retainAll(source_selected);
    source_selected.removeAll(src);
    src.addAll(source_selected);
  }

  public void updateDestinations(Set destinationsStrings, String masksString, SubjectList allSubjects, SME defaultSme)
          throws AdminException
  {
    DestinationList list = new DestinationList();
    for (Iterator i = destinationsStrings.iterator(); i.hasNext(); )
    {
      String token = (String) i.next();
      list.add(new Destination(allSubjects.get(token)));
    }

    MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();)
    {
      Mask m = (Mask) i.next();
      list.add(new Destination(m, defaultSme));
    }

    dst.retainAll(list);
    list.removeAll(dst);
    dst.addAll(list);
  }

  public boolean isEnabling()
  {
    return enabling;
  }

  public void setEnabling(boolean enabling)
  {
    this.enabling = enabling;
  }

  public boolean isBilling()
  {
    return billing;
  }

  public void setBilling(boolean billing)
  {
    this.billing = billing;
  }

  public boolean isArchiving()
  {
    return archiving;
  }

  public void setArchiving(boolean archiving)
  {
    this.archiving = archiving;
  }

  public PrintWriter store(PrintWriter out)
  {
    out.println("  <route id=\"" + StringEncoderDecoder.encode(getName()) + "\" billing=\"" + isBilling()
                + "\" archiving=\"" + isArchiving() + "\" enabling=\"" + isEnabling() + "\">");
    getSources().store(out);
    getDestinations().store(out);
    out.println("  </route>");
    return out;
  }
}
