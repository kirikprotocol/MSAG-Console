/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:43:03
 */
package ru.novosoft.smsc.admin.route;

import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.Random;


public class Route
{
  private String name = null;
  private SourceList src = null;
  private DestinationList dst = null;
  private boolean enabling = true;

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

  public void updateSources(String sourcesString, String masksString, SubjectList allSubjects)
  {
    SourceList source_selected = new SourceList();
    for (StringTokenizer tokenizer = new StringTokenizer(sourcesString, "\"", false);
         tokenizer.hasMoreTokens();) {
      String token = tokenizer.nextToken();
      source_selected.add(new Source(allSubjects.get(token)));
    }

    MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();) {
      Mask m = (Mask) i.next();
      source_selected.add(new Source(m));
    }

    src.retainAll(source_selected);
    source_selected.removeAll(src);
    src.addAll(source_selected);
  }

  public void updateDestinations(String destinationsString, String masksString, SubjectList allSubjects, SME defaultSme)
  {
    DestinationList list = new DestinationList();
    for (StringTokenizer tokenizer = new StringTokenizer(destinationsString, "\"", false);
         tokenizer.hasMoreTokens();) {
      String token = tokenizer.nextToken();
      list.add(new Destination(allSubjects.get(token)));
    }

    MaskList masks = new MaskList(masksString);
    for (Iterator i = masks.iterator(); i.hasNext();) {
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
}
