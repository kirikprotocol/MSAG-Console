/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.status;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Calendar;

/**
 * The <code>StatMessage</code> class represents
 * <p><p/>
 * Date: 27.01.2006
 * Time: 17:31:27
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class StatMessage {

    private String time;
    private String userLogin;
    private String category;
    private String message;

    SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    public StatMessage(final String userLogin, final String category, final String message) {
        this.userLogin = userLogin;
        this.category = category;
        this.message = message;
        Calendar calendar = Calendar.getInstance();
        this.time  = dateFormat.format(calendar.getTime());
    }


    public String getTime() {
        return time;
    }

    public void setTime(Date date) {
        this.time = dateFormat.format(date);
    }

    public String getUserLogin() {
        return userLogin;
    }

    public void setUserLogin(String userLogin) {
        this.userLogin = userLogin;
    }

    public String getCategory() {
        return category;
    }

    public void setCategory(String category) {
        this.category = category;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final StatMessage that = (StatMessage) o;

        if (category != null ? !category.equals(that.category) : that.category != null) return false;
        if (message != null ? !message.equals(that.message) : that.message != null) return false;
        if (time != null ? !time.equals(that.time) : that.time != null) return false;
        if (userLogin != null ? !userLogin.equals(that.userLogin) : that.userLogin != null) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = (time != null ? time.hashCode() : 0);
        result = 29 * result + (userLogin != null ? userLogin.hashCode() : 0);
        result = 29 * result + (category != null ? category.hashCode() : 0);
        result = 29 * result + (message != null ? message.hashCode() : 0);
        return result;
    }


    public String toString() {
        return "StatMessage{" +
                "message='" + message + '\'' +
                ", category='" + category + '\'' +
                ", userLogin='" + userLogin + '\'' +
                ", time='" + time + '\'' +
                '}';
    }
}
