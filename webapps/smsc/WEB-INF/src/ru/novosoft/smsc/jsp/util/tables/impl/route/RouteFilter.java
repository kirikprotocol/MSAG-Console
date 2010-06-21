package ru.novosoft.smsc.jsp.util.tables.impl.route;

/*
* Author: igork
* Date: 13.06.2002
* Time: 16:27:35
*/

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.*;


public class RouteFilter implements Filter {
    private Set src_subjects = null;
    private MaskList src_masks = null;
    private Set dst_subjects = null;
    private MaskList dst_masks = null;
    private List destSmes = null;
    private List srcSmes = null;
    private List names = null;
    private List providers = null;
    private List categories = null;
    private int intersection = 2; //Soft Filter ( old boolean intersection=false )
    private static final int ALLOWED = 0;
    private static final int NOT_ALLOWED = 1;
    private static final int UNKNOWN = 2;
    public static final int HARD = 0;
    public static final int HALFSOFT = 1;
    public static final int SOFT = 2;
    public static final int Subj = 1;
    public static final int Mask = 2;
    protected ProviderManager providerManager = null;
    protected CategoryManager categoryManager = null;


    public RouteFilter() {
        src_subjects = new HashSet();
        src_masks = new MaskList();
        dst_subjects = new HashSet();
        dst_masks = new MaskList();
        destSmes = new Vector();
        srcSmes = new Vector();
        names = new Vector();
        providers = new Vector();
        categories = new Vector();
    }

    public RouteFilter(int Intersection,
                       Set source_selected,
                       String src_masks,
                       Set destination_selected,
                       String dst_masks,
                       List destSmes, List names, List providers, List categories) {
        intersection = Intersection;
        // get sources
        this.src_subjects = source_selected;
        this.src_masks = new MaskList(src_masks);

        // get destinations
        this.dst_subjects = destination_selected;
        this.dst_masks = new MaskList(dst_masks);

        this.destSmes = destSmes;
        this.names = names;
        this.providers = providers;
        this.categories = categories;
    }

    public RouteFilter(int Intersection,
                       Set source_selected,
                       String src_masks,
                       Set destination_selected,
                       String dst_masks,
                       List destSmes, List names, List providers, List categories, ProviderManager providerManager, CategoryManager categoryManager) {
        intersection = Intersection;
        // get sources
        this.src_subjects = source_selected;
        this.src_masks = new MaskList(src_masks);

        // get destinations
        this.dst_subjects = destination_selected;
        this.dst_masks = new MaskList(dst_masks);

        this.destSmes = destSmes;
        this.names = names;
        this.providers = providers;
        this.categories = categories;
        this.providerManager = providerManager;
        this.categoryManager = categoryManager;

    }

    public boolean isEmpty() {
        return this.src_subjects.isEmpty() && this.src_masks.isEmpty()
                && this.dst_subjects.isEmpty() && this.dst_masks.isEmpty()
                && this.destSmes.isEmpty() && this.srcSmes.isEmpty() && this.names.isEmpty() && this.providers.isEmpty() && this.categories.isEmpty();
    }

    protected static int isSubjectAllowed(Set subjects, String subj) {
        if (subjects.isEmpty())
            return UNKNOWN;
        if (subjects.contains(subj))
            return ALLOWED;
        else {
            for (Iterator i = subjects.iterator(); i.hasNext();) {
                String subject = (String) i.next();
                if (subject.startsWith("+")) {
                    if (subj.equals(subject.substring(1)))
                        return ALLOWED;
                } else if (subj.toLowerCase().indexOf(subject.toLowerCase()) != -1)
                    return ALLOWED;
            }
        }
        return NOT_ALLOWED;
    }


    protected static int isMaskAllowed(MaskList masksSet, String maskStr) {
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

    protected static int isSourceAllowed(Set subjects, MaskList masks, int path, Source src) {
        if (path == 0) {
            if (src.isSubject())
                return isSubjectAllowed(subjects, src.getName());
            else
                return isMaskAllowed(masks, src.getName());
        } else {
            if (path == 1)
                return isSubjectAllowed(subjects, src.getName());
            if (path == 2)
                return isMaskAllowed(masks, src.getName());
        }
        return UNKNOWN;
    }

    protected int isSourcesAllowed(SourceList srcs, int path) {
        if (src_subjects.isEmpty() && src_masks.isEmpty())
            return UNKNOWN;

        for (Iterator i = srcs.iterator(); i.hasNext();) {
            switch (isSourceAllowed(src_subjects, src_masks, path, (Source) i.next())) {
                case ALLOWED:
                    if (intersection > 0)
                        return ALLOWED;
                    break;

                case NOT_ALLOWED:
                    if (intersection == 0)
                        return NOT_ALLOWED;
                    break;

                case UNKNOWN:
                    // do nothing
                    break;

                default:
                    throw new InternalError("unknown state");
            }
        }
        return intersection == 0 ? ALLOWED : NOT_ALLOWED;
    }

    protected int isDestinationsAllowed(DestinationList dsts, int path) {
        if (dst_subjects.isEmpty() && dst_masks.isEmpty())
            return UNKNOWN;

        for (Iterator i = dsts.iterator(); i.hasNext();) {
            switch (isSourceAllowed(dst_subjects, dst_masks, path, (Source) i.next())) {
                case ALLOWED:
                    if (intersection > 0)
                        return ALLOWED;
                    break;

                case NOT_ALLOWED:
                    if (intersection == 0)
                        return NOT_ALLOWED;
                    break;

                case UNKNOWN:
                    // do nothing
                    break;

                default:
                    throw new InternalError("unknown state");
            }
        }
        return intersection == 0 ? ALLOWED : NOT_ALLOWED;
    }

    protected int isSMEsAllowed(DestinationList dsts) {
        if (destSmes.isEmpty())
            return UNKNOWN;

        for (Iterator i = dsts.iterator(); i.hasNext();) {
            Destination dst = (Destination) i.next();
            if (intersection == 0) {

                if (!destSmes.contains(dst.getSme().getId()) && !destSmes.contains(dst.getSme().getId().toLowerCase()))

                    return NOT_ALLOWED;
                else {
                    for (Iterator j = destSmes.iterator(); j.hasNext();) {
                        String subject = (String) j.next();
                        if (dst.getSme().getId().toLowerCase().indexOf(subject.toLowerCase()) == -1)
                            return NOT_ALLOWED;
                    }
                }
            } else {
                if (destSmes.contains(dst.getSme().getId()) || destSmes.contains(dst.getSme().getId().toLowerCase()))
                    return ALLOWED;
                else {
                    for (Iterator j = destSmes.iterator(); j.hasNext();) {
                        String subject = (String) j.next();
                        if (dst.getSme().getId().toLowerCase().indexOf(subject.toLowerCase()) != -1)
                            return ALLOWED;
                    }
                }
            }
        }
        return intersection == 0 ? ALLOWED : NOT_ALLOWED;
    }

    protected int isNamesAllowed(String name) {
        if (names.isEmpty())
            return UNKNOWN;

        if (names.contains(name) || names.contains(name.toLowerCase()) || names.contains(name.toUpperCase()))
            return ALLOWED;

        for (Iterator j = names.iterator(); j.hasNext();) {
            String subject = (String) j.next();
            if (name.toLowerCase().indexOf(subject.toLowerCase()) != -1)
                return ALLOWED;
        }
        return NOT_ALLOWED;
    }

    protected int isProvidersAllowed(Long providerId) {
        if (providers.isEmpty())
            return UNKNOWN;
        if (providerId.longValue() == -1)
            return UNKNOWN;
        Provider provider = providerManager.getProvider(providerId);
        String name = provider.getName();
        if (providers.contains(name) || providers.contains(name.toLowerCase()) || providers.contains(name.toUpperCase()))
            return ALLOWED;

        for (Iterator j = providers.iterator(); j.hasNext();) {
            String subject = (String) j.next();
            if (name.toLowerCase().indexOf(subject.toLowerCase()) != -1)
                return ALLOWED;
        }
        return NOT_ALLOWED;
    }

    protected int isCategoriesAllowed(Long categoryId) {
        if (categories.isEmpty())
            return UNKNOWN;
        if (categoryId.longValue() == -1)
            return UNKNOWN;
        Category category = categoryManager.getCategory(categoryId);
        String name = category.getName();
        if (categories.contains(name) || categories.contains(name.toLowerCase()) || categories.contains(name.toUpperCase()))
            return ALLOWED;

        for (Iterator j = categories.iterator(); j.hasNext();) {
            String subject = (String) j.next();
            if (name.toLowerCase().indexOf(subject.toLowerCase()) != -1)
                return ALLOWED;
        }
        return NOT_ALLOWED;
    }

    protected int isSrcSmeAllowed(String srcSmeId) {
      if (srcSmes.isEmpty())
        return UNKNOWN;

      if (srcSmeId != null) {
        for (Iterator iter = srcSmes.iterator(); iter.hasNext();) {
          String sme = (String) iter.next();
          if(srcSmeId.toLowerCase().indexOf(sme) != -1)
            return ALLOWED;
        }
      }
      return NOT_ALLOWED;
    }

    public boolean isItemAllowed(DataItem item) {
        if (isEmpty())
            return true;
        String name = (String) item.getValue("Route ID");
        Long providerId = (Long) item.getValue("providerId");
        Long categoryId = (Long) item.getValue("categoryId");
        SourceList srcs = (SourceList) item.getValue("sources");
        DestinationList dsts = (DestinationList) item.getValue("destinations");
        String srcSmeId = (String) item.getValue("srcSmeId");

        switch (intersection) {
            case 0:

                return isSourcesAllowed(srcs, 0) != NOT_ALLOWED
                        && isDestinationsAllowed(dsts, 0) != NOT_ALLOWED
                        && isSMEsAllowed(dsts) != NOT_ALLOWED
                        && isSrcSmeAllowed(srcSmeId) != NOT_ALLOWED
                        && isNamesAllowed(name) != NOT_ALLOWED
                        && isProvidersAllowed(providerId) == ALLOWED
                        && isCategoriesAllowed(categoryId) == ALLOWED;

            case 1:

                return isSourcesAllowed(srcs, Subj) == ALLOWED
                        || isDestinationsAllowed(dsts, Subj) == ALLOWED
                        || isSourcesAllowed(srcs, Mask) == ALLOWED
                        || isDestinationsAllowed(dsts, Mask) == ALLOWED
                        || isSMEsAllowed(dsts) == ALLOWED
                        || isSrcSmeAllowed(srcSmeId) == ALLOWED
                        || isNamesAllowed(name) == ALLOWED
                        || isProvidersAllowed(providerId) == ALLOWED
                        || isCategoriesAllowed(categoryId) == ALLOWED;

            case 2:

                return isSourcesAllowed(srcs, 0) == ALLOWED
                        || isDestinationsAllowed(dsts, 0) == ALLOWED
                        || isSMEsAllowed(dsts) == ALLOWED
                        || isSrcSmeAllowed(srcSmeId) == ALLOWED
                        || isNamesAllowed(name) == ALLOWED
                        || isProvidersAllowed(providerId) == ALLOWED
                        || isCategoriesAllowed(categoryId) == ALLOWED;

            default:
                return true;
        }


    }


    public String[] getSourceSubjectNames() {
        return (String[]) src_subjects.toArray(new String[0]);
    }

    public String[] getSourceMaskStrings() {
        return (String[]) src_masks.getNames().toArray(new String[0]);
    }

    public String[] getDestinationSubjectNames() {
        return (String[]) dst_subjects.toArray(new String[0]);
    }

    public String[] getDestinationMaskStrings() {
        return (String[]) dst_masks.getNames().toArray(new String[0]);
    }

    public String[] getDestSmeIds() {
        return (String[]) destSmes.toArray(new String[0]);
    }

    public String[] getSrcSmeIds() {
      return (String[]) srcSmes.toArray(new String[0]);
    }

    public String[] getNames() {
        return (String[]) names.toArray(new String[0]);
    }

    public String[] getProviders() {
        return (String[]) providers.toArray(new String[0]);
    }

    public String[] getCategories() {
        return (String[]) categories.toArray(new String[0]);
    }

    public int getIntersection() {
        return intersection;
    }

    public boolean isIntersection() {
        if (intersection == 0)
            return true;
        else
            return false;
    }

    public void setSourceSubjectNames(String[] srcSubjs) {
        this.src_subjects = new HashSet(Arrays.asList(srcSubjs));
    }

    public void setSourceMaskStrings(String[] srcMasks) throws AdminException {
        this.src_masks = new MaskList(srcMasks);
    }

    public void setDestinationSubjectNames(String[] dstSubjs) {
        this.dst_subjects = new HashSet(Arrays.asList(dstSubjs));
    }

    public void setDestinationMaskStrings(String[] dstMasks) throws AdminException {
        this.dst_masks = new MaskList(dstMasks);
    }

    public void setDestSmeIds(String[] smes) {
        this.destSmes = Arrays.asList(smes);
    }

    public void setSrcSmeIds(String[] smes) {
        this.srcSmes = Arrays.asList(smes);
    }

    public void setNames(String[] names) {
        this.names = Arrays.asList(names);
    }

    public void setProviders(String[] providers) {
        this.providers = Arrays.asList(providers);
    }

    public void setCategories(String[] categories) {
        this.categories = Arrays.asList(categories);
    }

    public void setSelectSmes(String[] names) {
        this.names = Arrays.asList(names);
    }

    public void setIntersection(int intersection) {
        this.intersection = intersection;
    }

    public void setCategoryManager(CategoryManager categoryManager) {
        this.categoryManager = categoryManager;
    }

    public void setProviderManager(ProviderManager providerManager) {
        this.providerManager = providerManager;
    }
}
