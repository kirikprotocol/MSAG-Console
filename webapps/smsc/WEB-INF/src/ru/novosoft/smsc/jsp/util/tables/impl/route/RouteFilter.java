/*
* Author: igork
* Date: 13.06.2002
* Time: 16:27:35
*/
package ru.novosoft.smsc.jsp.util.tables.impl.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.*;


public class RouteFilter implements Filter
{
  private Set src_subjects = null;
  private MaskList src_masks = null;
  private Set dst_subjects = null;
  private MaskList dst_masks = null;
  private List smes = null;
  private List names = null;
  private int intersection = 2; //Soft Filter ( old boolean intersection=false )
  private static final int ALLOWED = 0;
  private static final int NOT_ALLOWED = 1;
  private static final int UNKNOWN = 2;
  public  static final int HARD= 0;
  public  static final int HALFSOFT= 1;
  public  static final int SOFT= 2;
  public  static final int Subj= 1;
  public  static final int Mask= 2;


  public RouteFilter()
  {
    src_subjects = new HashSet();
    src_masks = new MaskList();
    dst_subjects = new HashSet();
    dst_masks = new MaskList();
    smes = new Vector();
    names = new Vector();
  }

  public RouteFilter(int Intersection,
                     Set source_selected,
                     String src_masks,
                     Set destination_selected,
                     String dst_masks,
                     List smes,List names)
  {
    intersection = Intersection;
    // get sources
    this.src_subjects = source_selected;
    this.src_masks = new MaskList(src_masks);

    // get destinations
    this.dst_subjects = destination_selected;
    this.dst_masks = new MaskList(dst_masks);

    this.smes = smes;
    this.names = names;
  }

  public boolean isEmpty()
  {
    return this.src_subjects.isEmpty() && this.src_masks.isEmpty()
            && this.dst_subjects.isEmpty() && this.dst_masks.isEmpty()
            && this.smes.isEmpty()&& this.names.isEmpty();
  }

  protected static int isSubjectAllowed(Set subjects, String subj)
  {
    if (subjects.isEmpty())
      return UNKNOWN;
    if (subjects.contains(subj))
      return ALLOWED;
    else
    {
      for (Iterator i = subjects.iterator(); i.hasNext();) {
        String subject =  (String) i.next();
        if (subj.toLowerCase().indexOf(subject.toLowerCase()) != -1)
          return ALLOWED;
      }
    }
    return NOT_ALLOWED;
  }


  protected static int isMaskAllowed(MaskList masksSet, String maskStr)
  {
    if (masksSet.isEmpty())
      return UNKNOWN;

    final Mask maskToTest;
    try {
      maskToTest = new Mask(maskStr);
    } catch (AdminException e) {
      return UNKNOWN;
    }

    for (Iterator i = masksSet.iterator(); i.hasNext();) {
      Mask m = (Mask) i.next();
      if (m.addressConfirm(maskToTest))
        return ALLOWED;
    }
    return NOT_ALLOWED;
  }

  protected static int isSourceAllowed(Set subjects, MaskList masks,int path, Source src)
  {
   if (path==0){
    if (src.isSubject())
      return isSubjectAllowed(subjects, src.getName());
    else
      return isMaskAllowed(masks, src.getName());
   }
    else
   {
      if (path==1)
      return isSubjectAllowed(subjects, src.getName());
   if (path==2)
      return isMaskAllowed(masks, src.getName());
   }
    return UNKNOWN;
  }

  protected int isSourcesAllowed(SourceList srcs,int path)
  {
    if (src_subjects.isEmpty() && src_masks.isEmpty())
      return UNKNOWN;

    for (Iterator i = srcs.iterator(); i.hasNext();) {
      switch (isSourceAllowed(src_subjects, src_masks,path, (Source) i.next())) {
        case ALLOWED:
          if (intersection>0)
            return ALLOWED;
          break;

        case NOT_ALLOWED:
          if (intersection==0)
            return NOT_ALLOWED;
          break;

        case UNKNOWN:
          // do nothing
          break;

        default:
          throw new InternalError("unknown state");
      }
    }
    return intersection==0 ? ALLOWED : NOT_ALLOWED;
  }

  protected int isDestinationsAllowed(DestinationList dsts,int path)
  {
    if (dst_subjects.isEmpty() && dst_masks.isEmpty())
      return UNKNOWN;

    for (Iterator i = dsts.iterator(); i.hasNext();) {
      switch (isSourceAllowed(dst_subjects, dst_masks,path, (Source) i.next())) {
        case ALLOWED:
          if (intersection>0)
            return ALLOWED;
          break;

        case NOT_ALLOWED:
          if (intersection==0)
            return NOT_ALLOWED;
          break;

        case UNKNOWN:
          // do nothing
          break;

        default:
          throw new InternalError("unknown state");
      }
    }
    return intersection==0 ? ALLOWED : NOT_ALLOWED;
  }

  protected int isSMEsAllowed(DestinationList dsts)
  {
    if (smes.isEmpty())
      return UNKNOWN;

    for (Iterator i = dsts.iterator(); i.hasNext();) {
      Destination dst = (Destination) i.next();
      if (intersection==0) {

        if (!smes.contains(dst.getSme().getId()) && !smes.contains(dst.getSme().getId().toLowerCase()))

          return NOT_ALLOWED;
        else
        {
          for (Iterator j = smes.iterator(); j.hasNext();) {
            String subject =  (String) j.next();
            if (dst.getSme().getId().toLowerCase().indexOf(subject.toLowerCase()) == -1)
              return NOT_ALLOWED;
          }
        }
      } else {
        if (smes.contains(dst.getSme().getId()) || smes.contains(dst.getSme().getId().toLowerCase()))
          return ALLOWED;
        else
        {
          for (Iterator j = smes.iterator(); j.hasNext();) {
            String subject =  (String) j.next();
            if (dst.getSme().getId().toLowerCase().indexOf(subject.toLowerCase()) != -1)
              return ALLOWED;
          }
        }
      }
    }
    return intersection==0 ? ALLOWED : NOT_ALLOWED;
  }

  protected int isNamesAllowed(String name)
  {
    if (names.isEmpty())
      return UNKNOWN;

    if (names.contains(name) || names.contains(name.toLowerCase()) || names.contains(name.toUpperCase()) )
      return ALLOWED;

    for (Iterator j = names.iterator(); j.hasNext();) {
      String subject =  (String) j.next();
      if (name.indexOf(subject) != -1)
        return ALLOWED;
    }
    return NOT_ALLOWED;
  }
  public boolean isItemAllowed(DataItem item)
  {
    if (isEmpty())
      return true;
    String name =(String)item.getValue("Route ID");
    SourceList srcs = (SourceList) item.getValue("sources");
    DestinationList dsts = (DestinationList) item.getValue("destinations");

    switch (intersection) {
      case 0:

        return isSourcesAllowed(srcs,0) != NOT_ALLOWED
                && isDestinationsAllowed(dsts,0) != NOT_ALLOWED
                && isSMEsAllowed(dsts) != NOT_ALLOWED
                && isNamesAllowed(name) != NOT_ALLOWED;

      case 1:

        return ( src_subjects.isEmpty()|| isSourcesAllowed(srcs,Subj) == ALLOWED
                || isDestinationsAllowed(dsts,Subj) == ALLOWED )
                && ( src_masks.isEmpty()|| isSourcesAllowed(srcs,Mask) == ALLOWED
                || isDestinationsAllowed(dsts,Mask) == ALLOWED )
                && ( smes.isEmpty() ||  isSMEsAllowed(dsts) == ALLOWED)
                && ( names.isEmpty() || isNamesAllowed(name) == ALLOWED);

      case 2:

        return isSourcesAllowed(srcs,0) == ALLOWED
                || isDestinationsAllowed(dsts,0) == ALLOWED
                || isSMEsAllowed(dsts) == ALLOWED
                || isNamesAllowed(name) == ALLOWED;

      default:
        return true;
    }


  }


  public String[] getSourceSubjectNames()
  {
    return (String[]) src_subjects.toArray(new String[0]);
  }

  public String[] getSourceMaskStrings()
  {
    return (String[]) src_masks.getNames().toArray(new String[0]);
  }

  public String[] getDestinationSubjectNames()
  {
    return (String[]) dst_subjects.toArray(new String[0]);
  }

  public String[] getDestinationMaskStrings()
  {
    return (String[]) dst_masks.getNames().toArray(new String[0]);
  }

  public String[] getSmeIds()
  {
    return (String[]) smes.toArray(new String[0]);
  }

  public String[] getNames()
  {
    return (String[]) names.toArray(new String[0]);
  }

  public int getIntersection()
  {
    return intersection;
  }
     public boolean isIntersection()
  {
      if (intersection==0) return true;
       else return false;
  }
  public void setSourceSubjectNames(String[] srcSubjs)
  {
    this.src_subjects = new HashSet(Arrays.asList(srcSubjs));
  }

  public void setSourceMaskStrings(String[] srcMasks) throws AdminException
  {
    this.src_masks = new MaskList(srcMasks);
  }

  public void setDestinationSubjectNames(String[] dstSubjs)
  {
    this.dst_subjects = new HashSet(Arrays.asList(dstSubjs));
  }

  public void setDestinationMaskStrings(String[] dstMasks) throws AdminException
  {
    this.dst_masks = new MaskList(dstMasks);
  }
       
  public void setSmeIds(String[] smes)
  {
    this.smes = Arrays.asList(smes);
  }
  public void setNames(String[] names)
  {
    this.names = Arrays.asList(names);
  }
  public void setSelectSmes(String[] names)
  {
    this.names = Arrays.asList(names);
  }
  public void setIntersection(int intersection)
  {
    this.intersection = intersection;
  }


}
