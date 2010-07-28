package ru.sibinco.scag.backend.sme;

import ru.sibinco.lib.backend.sme.SmeStatus;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.scag.backend.SCAGAppContext;

import java.util.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 06.04.2005
 * Time: 16:26:00
 * To change this template use File | Settings | File Templates.
 */
public class SmeStatusFunctions {

    public static String showSmeStatus(SmeStatus status) {
        String result = "";
        if (status != null) {
            if (status.isInternal()) {
                result += "><img src=\"content/images/ic_internal.gif\" width=10 height=10 title='internal'></span>";
            } else {
                if (!status.isConnected()) {
                    result += "><img src=\"content/images/ic_disconnct.gif\" width=10 height=10 title='disconnected'></span>";
                } else {
                    switch (status.getBindMode()) {
                        case ru.sibinco.lib.backend.sme.Sme.MODE_TX:
                            result += " class=C080>TX&nbsp;" + status.getInAddress() + "</span>";
                            break;
                        case ru.sibinco.lib.backend.sme.Sme.MODE_RX:
                            result += " class=C080>RX&nbsp;" + status.getInAddress() + "</span>";
                            break;
                        case ru.sibinco.lib.backend.sme.Sme.MODE_TRX:
                            result += " class=C080>TRX&nbsp;" + status.getInAddress() + "</span>";
                            break;
                        default:
                            result += " class=C080>unknown&nbsp;" + status.getInAddress() + '/' + status.getOutAddress() + "</span>";
                    }
                }
            }
        } else {
            result += " class=C000>unknown</span>";
        }
        return result;
    }

    public static String smeStatus(SmeStatus status, String smeId) {
        Date date = new Date();
        long time = date.getTime();
        SimpleDateFormat format = new SimpleDateFormat();
        //System.out.println("getting sme status in " + format.format(date));
        String elem_id = "CONNECTION_STATUSERVICE_" + StringEncoderDecoder.encode(smeId);
        return "<span id=\"" + elem_id + "\" datasrc=#tdcConnStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(smeId) + "\" " + showSmeStatus(status);
    }
}
