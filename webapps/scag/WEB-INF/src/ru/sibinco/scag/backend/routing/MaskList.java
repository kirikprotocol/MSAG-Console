/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing;

import org.apache.log4j.Logger;

import java.util.*;
import java.io.PrintWriter;

import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>MaskList</code> class represents
 * <p><p/>
 * Date: 21.10.2005
 * Time: 11:25:51
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class MaskList {
    private Map masks = new HashMap();
    private Logger logger = Logger.getLogger(this.getClass().getName());

    public MaskList() {
    }

    public MaskList(String masks) {
        logger.debug("enter " + this.getClass().getName() + ".MaskList(" + masks + ")");
        for (StringTokenizer tokenizer = new StringTokenizer(masks); tokenizer.hasMoreTokens();) {
            final String mask = tokenizer.nextToken();
            try {
                add(new Mask(mask));
            } catch (SibincoException e) {
                logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
            }
        }
        logger.debug("exit " + this.getClass().getName() + ".MaskList(" + masks + ")");
    }

    public MaskList(String[] masks) throws SibincoException {
        logger.debug("enter " + this.getClass().getName() + ".MaskList(" + masks + ")");
        for (int i = 0; i < masks.length; i++) {
            String mask = masks[i].trim();
            try {
                add(new Mask(mask));
            } catch (SibincoException e) {
                logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
                throw e;
            }
        }
        logger.debug("exit " + this.getClass().getName() + ".MaskList(" + masks + ")");
    }

    public MaskList(Collection masks) throws SibincoException {
        logger.debug("enter " + this.getClass().getName() + ".MaskList(" + masks + ")");
        for (Iterator i = masks.iterator(); i.hasNext();) {
            String mask = ((String) i.next()).trim();
            try {
                add(new Mask(mask));
            } catch (SibincoException e) {
                logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
                throw e;
            }
        }
        logger.debug("exit " + this.getClass().getName() + ".MaskList(" + masks + ")");
    }

    public MaskList(Mask mask) {
        add(mask);
    }

    public Mask add(Mask newMask) {
        if (newMask == null)
            throw new NullPointerException("Mask is null");

        return (Mask) masks.put(newMask.getMask(), newMask);
    }

    public Mask get(String key) {
        return (Mask) masks.get(key);
    }

    public int size() {
        return masks.size();
    }

    public Iterator iterator() {
        return masks.values().iterator();
    }

    public String getText() {
        String result = "";
        for (Iterator i = masks.keySet().iterator(); i.hasNext();) {
            result += (String) i.next() + "<br>";
        }
        return result;
    }

    public SortedList getNames() {
        return new SortedList(masks.keySet());
    }

    public PrintWriter store(PrintWriter out) {
        TreeSet<Mask> sortedMasks = new TreeSet<Mask>();
        for (Iterator i = iterator(); i.hasNext();) {
            sortedMasks.add((Mask) i.next());
        }

        for (Mask mask: sortedMasks) mask.store(out);
        return out;
    }

    public PrintWriter store(PrintWriter out, String param) {
        for (Iterator i = iterator(); i.hasNext();) {
            ((Mask) i.next()).store(out, param);
        }
        return out;
    }

    public boolean isEmpty() {
        return masks.size() == 0;
    }

    public void clear() {
        masks.clear();
    }

    public static String[] normalizeMaskList(String[] masks) throws SibincoException {
        return (String[]) new MaskList(masks).getNames().toArray(new String[0]);
    }

    public boolean contains(Mask mask) {
        Mask m = get(mask.getMask());
        return m != null && m.equals(mask);
    }

    public String toString(){
        String result ="MaskList{";
        for(Object key: masks.keySet()){
            Mask mask = (Mask) masks.get(key);
            result = result+mask.getMask()+",";
        }
        if (result.endsWith(",")) result = result.substring(0, result.length()-1);

        result = result+"}";
        return result;
    }
}
