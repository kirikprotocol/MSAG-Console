/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 16:27:35
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.admin.AdminException;
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
	}

	public boolean isEmpty()
	{
		return this.src_subjects.isEmpty() && this.src_masks.isEmpty()
				  && this.dst_subjects.isEmpty() && this.dst_masks.isEmpty()
				  && this.smes.isEmpty();
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
		if (isEmpty())
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


	public String[] getSourceSubjectNames()
	{
		return (String[])src_subjects.toArray(new String[0]);
	}

	public String[] getSourceMaskStrings()
	{
		return (String[])src_masks.getNames().toArray(new String[0]);
	}

	public String[] getDestinationSubjectNames()
	{
		return (String[])dst_subjects.toArray(new String[0]);
	}

	public String[] getDestinationMaskStrings()
	{
		return (String[])dst_masks.getNames().toArray(new String[0]);
	}

	public String[] getSmeIds()
	{
		return (String[])smes.toArray(new String[0]);
	}

	public boolean isIntersection()
	{
		return intersection;
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

	public void setIntersection(boolean intersection)
	{
		this.intersection = intersection;
	}
}
