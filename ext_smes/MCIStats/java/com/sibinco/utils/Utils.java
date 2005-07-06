/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package com.sibinco.utils;

import java.util.Calendar;
import java.text.ParseException;
import java.text.SimpleDateFormat;

/**
 * The <code>Utils</code> class represents utilities
 * <p><p/>
 * Date: 22.06.2005
 * Time: 18:13:31
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Utils {

    private static final Calendar calendar = Calendar.getInstance();
    public static final String DATE_FORMAT_FULL = "yyyy.MM.dd.HH.ss";
    public static final String DATE_FORMAT_SHORT_YEAR = "yy.MM.dd.HH.ss";
    public static final String DATE_FORMAT_DAY_MONTH_YEAR = "dd.MM.yy";
    public static final String DATE_FORMAT_DAY_MONTH_FULL_YEAR = "dd.MM.yyyy";
    public static final String DATE_FORMAT_MONTH_DAY = "MM/dd";
    public static final String DATE_FORMAT_YEAR_MONTH_DAY = "yyyy-MM-dd";
    public static final long ONE_MINUTE = 1000 * 60;
    public static final long ONE_HOUR = ONE_MINUTE * 60;
    public static final long ONE_DAY = ONE_HOUR * 24;


    /**
     * ��������� �����������, ��������� �������� ���������� ������
     */
    private Utils() {
    }

    /**
     * ����� ���������� ���������� ������ � ������
     *
     * @param text    - ������ ��� ���������
     * @param find    - ������� ������
     * @param replace - ������ ��� ������
     * @return String - ������������ ������
     */
    public static String replace(String text, String find, String replace) {
        // ����� ���������� � ����� ������, ��� �������� ������������� ��������
        if (text == null || find == null || replace == null) {
            return text;
        }
        int fLength = find.length();
        StringBuffer myString = new StringBuffer(text);
        int hit = text.lastIndexOf(find);
        while (hit != -1) {
            myString.replace(hit, hit + fLength, replace);
            hit = text.lastIndexOf(find, hit - fLength);
        }
        return myString.toString();
    }


    /**
     * ������� ��������� ����, �������������� � ���� ������, ��������� �������� ������ ����
     *
     * @param dateString - ���� � ���� ������
     * @param pattern    - ������ ������������� ���� � ���� ������
     * @return java.util.Date - ����������� �� ������ ����
     * @throws java.text.ParseException -  ���������� ���������� ����
     */
    public static java.util.Date parseDate(String dateString, String pattern) throws
            ParseException {
        if (dateString == null) {
            return null;
        }
        SimpleDateFormat dateFormat = new SimpleDateFormat(pattern);
        return dateFormat.parse(dateString);
    }

    /**
     * ������� ����������� ���� � ������������ � ��������
     *
     * @param date    - ���� ��� ��������������
     * @param pattern - ������ ������ ����, ��. ������� ��� java.text.SimpleDateFormat
     * @return String - ����������������� ����
     */
    public static String formatDate(java.util.Date date, String pattern) {
        if (date == null) {
            return "";
        }
        SimpleDateFormat dateFormat = new SimpleDateFormat(pattern);
        return dateFormat.format(date);
    }

    public static String getBeforeDay() {
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMdd");
        java.util.Date currentDate = new java.util.Date();
        long time = currentDate.getTime() - Utils.ONE_DAY;
        currentDate = new java.util.Date(time);
        return  dateFormat.format(currentDate);
    }


}
