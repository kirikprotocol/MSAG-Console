/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package com.sibinco.logger;

import com.sibinco.utils.Utils;

import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.FileOutputStream;
import java.io.File;
import java.io.UnsupportedEncodingException;
import java.io.FileNotFoundException;
import java.util.Date;

/**
 * The <code>LogGenerator</code> represents utility class
 * <p><p/>
 * Date: 22.06.2005
 * Time: 18:10:28
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class LogGenerator {


    public static void main(String[] args) {

        generateLog("C:\\test\\logs");
    }

    public static void generateLog(String path_dir) {

        OutputStreamWriter fout = null;
        File dir = new File(path_dir);
        String path = dir.getAbsolutePath();
        int len = -1;
        len = path.lastIndexOf("/");

        if (len == -1) {
            len = path.lastIndexOf("\\");
        }
        //path = path.substring(0, len);
        String str = path + "/" + "24_06_2005" + ".log";
        File files = new File(str);

        try {
            fout = new OutputStreamWriter(new FileOutputStream(files), "Cp1251");
            PrintWriter fw = new PrintWriter(fout, true);


            for (int i = 0; i < 10000; i++) {
                long pin = ((long) (Math.random() * 10000000));

                long time = new Date().getTime();
                time += Utils.ONE_MINUTE * i;
                String date = Utils.formatDate(new Date(time), Utils.DATE_FORMAT_DAY_MONTH_YEAR);
                if (i % 2 == 0) {
                    date += " " + "D" + " " + "+7913" + pin;
                    fw.println(date);
                }else if (i % 3 == 0) {
                    date += " " + "M" + " " + "+7913" + pin;
                    fw.println(date);
                }
            }
            fw.println();
            fw.flush();

        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

}
