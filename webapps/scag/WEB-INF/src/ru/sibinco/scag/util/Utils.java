/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.util;

import javax.servlet.http.HttpServletRequest;

/**
 * The <code>Utils</code> class represents
 * <p><p/>
 * Date: 26.02.2006
 * Time: 11:27:21
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Utils {


    public static String getPath(final HttpServletRequest request) {
        String path = request.getServletPath();
        int pos = path.indexOf(".jsp");
        if (pos > 0) {
            int pos2 = path.lastIndexOf('/', pos) + 1;
            if (pos2 > 0)
                path = request.getContextPath() + path.substring(0, pos2);
        }
        return path;
    }
}

