/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package com.sibinco.logger;

import com.sibinco.utils.Utils;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.File;
import java.util.StringTokenizer;
import java.util.Date;
import java.util.regex.Pattern;

/**
 * The <code>LogReader</code> class represents
 * <p><p/>
 * Date: 23.06.2005
 * Time: 13:42:23
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class LogReader {

    public static final int LOG_INFO_PRPERTIES = 1;

    public static void main(String[] args) {

    }

    public static int getAbonents(String log_unic_abonents, Date date, String sign) {
        int result = 0;

        FileReader filereader = null;
        BufferedReader bufferedreader = null;
        //String dateString = Utils.formatDate(date, Utils.DATE_FORMAT_DAY_MONTH_YEAR);
        long time = date.getTime() - Utils.ONE_DAY;
        date = new Date(time);
        String line;
        String filename = log_unic_abonents + Utils.formatDate(date, Utils.DATE_FORMAT_YEAR_MONTH_DAY);

        File fileToRead = new File(filename);
        if (!fileToRead.exists()) {
            throw new NullPointerException("File: " + filename + " dosn't exist!");
        }
        try {
            filereader = new FileReader(fileToRead);
            bufferedreader = new BufferedReader(filereader);
            while ((line = bufferedreader.readLine()) != null) {
                line = line.substring(LOG_INFO_PRPERTIES);
                StringTokenizer st = new StringTokenizer(line);
                while (st.hasMoreTokens()) {
                    if (st.nextToken().equals(sign)) {
                        //System.out.println(st.nextToken());
                        result += 1;
                    }
                }
            }
            filereader.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return result;
    }

    public static int applyToServise(String fileToRead, Date date, String patternString) {
        int result = 0;
        long time = date.getTime() - Utils.ONE_DAY;
        date = new Date(time);

        String fileDate = Utils.formatDate(date, Utils.DATE_FORMAT_YEAR_MONTH_DAY);
        String fileName =  fileToRead + fileDate;
        File file = new File(fileName);
        if (!file.exists()) {
            throw new NullPointerException("File: " + fileName + " dosn't exist!");
        }
        FileReader filereader = null;
        BufferedReader bufferedreader = null;
        String line;

        try {
//          Pattern pattern = Pattern.compile("(?ix).\\s+\\d{2}/\\d{2}\\s+\\d{2}:\\d{2}:\\d{2}\\.\\d{3}\\s+\\(.*?\\)\\s+SessionManager\\s+Creating\\s+session:");
          Pattern pattern = Pattern.compile(patternString);
            filereader = new FileReader(file);
            bufferedreader = new BufferedReader(filereader);
            while ((line = bufferedreader.readLine()) != null) {
                if( pattern.matcher(line).matches() ) result++;
            }
            filereader.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return result;
    }

    private static String getFileNameForAbonents(Date date, String delimeter) {
        String result = "";
        String currentDate = Utils.formatDate(date, Utils.DATE_FORMAT_DAY_MONTH_FULL_YEAR);
        int length = currentDate.indexOf('.');
        String day = currentDate.substring(0, length);
        result = day + delimeter;
        length += 1;
        String tmp = currentDate.substring(length);
        length = currentDate.indexOf('.');
        String month = tmp.substring(0, length);
        result += month + delimeter;

        length += 1;
        tmp = currentDate.substring(length);
        length = currentDate.indexOf('.');
        length += 1;
        String year = tmp.substring(length);
        result += year;

        return result;
    }
}
