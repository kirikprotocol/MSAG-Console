<%@ page import="ru.novosoft.smsc.admin.route.SME,
                 ru.novosoft.smsc.admin.route.SmeStatus" %><%!
    String showSmeStatus(SmeStatus status) {
        String result = "";
        if (status != null) {
            if (status.isInternal()) {
                result += "><img src=\"/images/ic_internal.gif\" width=10 height=10 title='" + getLocString("common.hints.internal") + "'></span>";
            } else {
                if (!status.isConnected()) {
                    result += "><img src=\"/images/ic_disconnct.gif\" width=10 height=10 title='" + getLocString("common.hints.disconnected") + "'></span>";
                } else {
                    switch (status.getBindMode()) {
                        case SME.MODE_TX:
                            result += " class=C080>TX&nbsp;" + status.getInAddress() + "</span>";
                            break;
                        case SME.MODE_RX:
                            result += " class=C080>RX&nbsp;" + status.getInAddress() + "</span>";
                            break;
                        case SME.MODE_TRX:
                            final String inAddress = status.getInAddress();
                            final String outAddress = status.getOutAddress();
                            result += " class=C080>TRX&nbsp;" + (inAddress.equals(outAddress) ? inAddress : inAddress + "/" + outAddress) + "</span>";
                            break;
                        default:
                            result += " class=C080>" + getLocString("common.statuses.unknown") + "&nbsp;" + status.getInAddress() + '/' + status.getOutAddress() + "</span>";
                    }
                }
            }
        } else {
            result += " class=C000>" + getLocString("common.statuses.unknown") + "</span>";
        }
        return result;
    }%>