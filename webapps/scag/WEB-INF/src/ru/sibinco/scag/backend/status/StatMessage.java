/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.status;

import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.Date;

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

    SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");

    public StatMessage(String userLogin, String category, String message) {
        this.userLogin = userLogin;
        this.category = category;
        this.message = message;
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

}
