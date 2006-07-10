/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http.placement;

import ru.sibinco.lib.SibincoException;


/**
 * The <code>AbonentsPlacement</code> class represents
 * <p><p/>
 * <p/>
 * Date: 06.07.2006
 * Time: 17:22:21
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class AbonentsPlacement extends Placement{
    private int priority;

    public AbonentsPlacement(final String type, final String name, final int priority) throws SibincoException {
        super(type, name);
        this.priority = priority;
    }


    public int getPriority() {
        return priority;
    }

    public void setPriority(int priority) {
        this.priority = priority;
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        if (!super.equals(o)) return false;

        final AbonentsPlacement that = (AbonentsPlacement) o;

        if (priority != that.priority) return false;

        return true;
    }

    public int hashCode() {
        int result = super.hashCode();
        result = 29 * result + priority;
        return result;
    }


    public String toString() {
        return "AbonentsPlacement{" +
                "type='" + getType() + '\'' +
                ", name='" + getName() + '\'' +
                "priority=" + priority +
                '}';
    }
}
