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
     * приватный конструктор, запрещает создание экземпляра класса
     */
    private Utils() {
    }

    /**
     * Метод производит глобальную замену в строке
     *
     * @param text    - строка для обработки
     * @param find    - искомая строка
     * @param replace - строка для замены
     * @return String - обработанная строка
     */
    public static String replace(String text, String find, String replace) {
        // поиск происходит с конца строки, без создания промежуточных обьектов
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
     * Функция разбирает дату, представленную в виде строки, используя заданный шаблон даты
     *
     * @param dateString - дата в виде строки
     * @param pattern    - шаблон представления даты в виде строки
     * @return java.util.Date - разобранная из строки дата
     * @throws java.text.ParseException -  исключения разборщика даты
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
     * Функция форматирует дату в соовтетствии с шаблоном
     *
     * @param date    - дата для форматирования
     * @param pattern - шаблон вывода даты, см. шаблоны для java.text.SimpleDateFormat
     * @return String - отформатированная дата
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
