/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 16:27:35
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import javax.servlet.http.HttpSession;
import java.util.*;


public class RouteFilter implements Filter
{
  private boolean empty = true;
  private Set src_subjects = null;
  private MaskList src_masks = null;
  private Set dst_subjects = null;
  private MaskList dst_masks = null;
  private List smes = null;
  private boolean intersection = false;
  private static final int ALLOWED = 0;
  private static final int NOT_ALLOWED = 1;
  private static final int UNKNOWN = 2;

  public RouteFilter()
  {
    src_subjects = new HashSet();
    src_masks = new MaskList();
    dst_subjects = new HashSet();
    dst_masks = new MaskList();
    smes = new Vector();
    empty = true;
  }

  public RouteFilter(boolean isIntersection,
                     Set source_selected,
                     String src_masks,
                     Set destination_selected,
                     String dst_masks,
                     List smes,
                     SubjectList allSubjects)
  {
    intersection = isIntersection;
    // get sources
    this.src_subjects = source_selected;
    this.src_masks = new MaskList(src_masks);

    // get destinations
    this.dst_subjects = destination_selected;
    this.dst_masks = new MaskList(dst_masks);

    this.smes = smes;

    empty = this.src_subjects.isEmpty() && this.src_masks.isEmpty()
            && this.dst_subjects.isEmpty() && this.dst_masks.isEmpty()
            && this.smes.isEmpty();
  }

  public boolean isEmpty()
  {
    HttpSession s;
    return empty;
  }

  protected static int isSubjectAllowed(Set subjects, String subj)
  {
    if (subjects.isEmpty())
      return UNKNOWN;
    if (subjects.contains(subj))
      return ALLOWED;
    else
      return NOT_ALLOWED;
  }

  protected static int isMaskAllowed(MaskList masksSet, String mask)
  {
    if (masksSet.isEmpty())
      return UNKNOWN;

    for (Iterator i = masksSet.iterator(); i.hasNext();)
    {
      Mask m = (Mask) i.next();
      if (mask.startsWith(m.getMask()))
        return ALLOWED;
    }
    return NOT_ALLOWED;
  }

  protected static int isSourceAllowed(Set subjects, MaskList masks, Source src)
  {
    if (src.isSubject())
      return isSubjectAllowed(subjects, src.getName());
    else
      return isMaskAllowed(masks, src.getName());
  }

  protected int isSourcesAllowed(SourceList srcs)
  {
    if (src_subjects.isEmpty() && src_masks.isEmpty())
      return UNKNOWN;

    for (Iterator i = srcs.iterator(); i.hasNext();)
    {
      switch (isSourceAllowed(src_subjects, src_masks, (Source) i.next()))
      {
        case ALLOWED:
          if (!intersection)
            return ALLOWED;
          break;

        case NOT_ALLOWED:
          if (intersection)
            return NOT_ALLOWED;
          break;

        case UNKNOWN:
          // do nothing
          break;

        default:
          throw new InternalError("unknown state");
      }
    }
    return intersection ? ALLOWED : NOT_ALLOWED;
  }

  protected int isDestinationsAllowed(DestinationList dsts)
  {
    if (dst_subjects.isEmpty() && dst_masks.isEmpty())
      return UNKNOWN;

    for (Iterator i = dsts.iterator(); i.hasNext();)
    {
      switch (isSourceAllowed(dst_subjects, dst_masks, (Source) i.next()))
      {
        case ALLOWED:
          if (!intersection)
            return ALLOWED;
          break;

        case NOT_ALLOWED:
          if (intersection)
            return NOT_ALLOWED;
          break;

        case UNKNOWN:
          // do nothing
          break;

        default:
          throw new InternalError("unknown state");
      }
    }
    return intersection ? ALLOWED : NOT_ALLOWED;
  }

  protected int isSMEsAllowed(DestinationList dsts)
  {
    if (smes.isEmpty())
      return UNKNOWN;

    for (Iterator i = dsts.iterator(); i.hasNext();)
    {
      Destination dst = (Destination) i.next();
      if (intersection)
      {
        if (!smes.contains(dst.getSme().getId()))
          return NOT_ALLOWED;
      }
      else
      {
        if (smes.contains(dst.getSme().getId()))
          return ALLOWED;
      }
    }
    return intersection ? ALLOWED : NOT_ALLOWED;
  }

  public boolean isItemAllowed(DataItem item)
  {
    if (empty)
      return true;

    SourceList srcs = (SourceList) item.getValue("sources");
    DestinationList dsts = (DestinationList) item.getValue("destinations");

    if (intersection)
    {
      return isSourcesAllowed(srcs) != NOT_ALLOWED
              && isDestinationsAllowed(dsts) != NOT_ALLOWED
              && isSMEsAllowed(dsts) != NOT_ALLOWED;
    }
    else
    {
      return isSourcesAllowed(srcs) == ALLOWED
              || isDestinationsAllowed(dsts) == ALLOWED
              || isSMEsAllowed(dsts) == ALLOWED;
    }
  }


  public Set getSourceSubjectNames()
  {
    if (empty)
      return new HashSet();
    else
      return src_subjects;
  }

  public Set getSourceMaskStrings()
  {
    if (empty)
      return new HashSet();
    else
      return src_masks.getNames();
  }

  public Set getDestinationSubjectNames()
  {
    if (empty)
      return new HashSet();
    else
      return dst_subjects;
  }

  public Set getDestinationMaskStrings()
  {
    if (empty)
      return new HashSet();
    else
      return dst_masks.getNames();
  }

  public List getSmeIds()
  {
    if (empty)
      return new Vector();
    else
      return smes;
  }

  public boolean isIntersection()
  {
    return intersection;
  }
}
