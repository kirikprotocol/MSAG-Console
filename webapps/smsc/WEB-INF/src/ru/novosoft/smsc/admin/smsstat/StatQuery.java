/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 1:41:57 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

import java.util.Date;

public class StatQuery
{
    private Date fromDate = new Date();
    private Date tillDate = new Date();

    private boolean fromDateEnabled = true;
    private boolean tillDateEnabled = false;

    public Date getFromDate() {
        return fromDate;
    }

    public Date getTillDate() {
        return tillDate;
    }

    public void setFromDate(Date fromDate) {
        this.fromDate = fromDate;
    }

    public void setTillDate(Date tillDate) {
        this.tillDate = tillDate;
    }

    public void setFromDateEnabled(boolean fromDateEnabled) {
        this.fromDateEnabled = fromDateEnabled;
    }

    public void setTillDateEnabled(boolean tillDateEnabled) {
        this.tillDateEnabled = tillDateEnabled;
    }

    public boolean isFromDateEnabled() {
        return fromDateEnabled;
    }

    public boolean isTillDateEnabled() {
        return tillDateEnabled;
    }
}
