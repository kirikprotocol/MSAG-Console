/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 27, 2002
 * Time: 4:45:21 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsview;

import java.util.Date;
import java.util.TimeZone;

public class DateConvertor
{
    public static Date convertLocalToGMT(Date date) {
        long time = date.getTime();
        return new Date(time - TimeZone.getDefault().getOffset(time));
    }
    public static Date convertGMTToLocal(Date gmt) {
        long time = gmt.getTime();
        return new Date(time + TimeZone.getDefault().getOffset(time));
    }
}
