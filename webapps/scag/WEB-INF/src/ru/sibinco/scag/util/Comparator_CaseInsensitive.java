/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.util;

import java.util.Comparator;


/**
 * The <code>Comparator_CaseInsensitive</code> class represents
 * <p><p/>
 * Date: 24.07.2006
 * Time: 12:48:07
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Comparator_CaseInsensitive implements Comparator {
    public int compare(Object o1, Object o2) {
        if (o1 instanceof String && o2 instanceof String) {
            String s1 = (String) o1;
            String s2 = (String) o2;
            return s1.compareToIgnoreCase(s2);
        }
        if (o1 instanceof Comparable)
            return ((Comparable) o1).compareTo(o2);
        if (o2 instanceof Comparable)
            return -((Comparable) o2).compareTo(o1);
        return 0;
    }
}
