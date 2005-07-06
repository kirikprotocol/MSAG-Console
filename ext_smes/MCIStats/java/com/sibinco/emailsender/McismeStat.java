/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package com.sibinco.emailsender;

/**
 * Created by IntelliJ IDEA.
 * User: igor
 * Date: May 31, 2005
 * Time: 5:34:19 PM
 * To change this template use File | Settings | File Templates.
 */
public class McismeStat {

    private String period;
    private String sumMissed;
    private String sumDelivered;
    private String sumFailed;
    private String sumNotified;

    public McismeStat() {
    }


    public String getPeriod() {
        return period;
    }

    public void setPeriod(String period) {
        this.period = period;
    }

    public String getSumMissed() {
        return sumMissed;
    }

    public void setSumMissed(String sumMissed) {
        this.sumMissed = sumMissed;
    }

    public String getSumDelivered() {
        return sumDelivered;
    }

    public void setSumDelivered(String sumDelivered) {
        this.sumDelivered = sumDelivered;
    }

    public String getSumFailed() {
        return sumFailed;
    }

    public void setSumFailed(String sumFailed) {
        this.sumFailed = sumFailed;
    }

    public String getSumNotified() {
        return sumNotified;
    }

    public void setSumNotified(String sumNotified) {
        this.sumNotified = sumNotified;
    }
}
