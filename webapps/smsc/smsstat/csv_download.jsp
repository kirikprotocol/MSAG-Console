<%@ page import="ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.jsp.smsstat.SmsStatFormBean,
                 java.text.DateFormat,
                 java.text.SimpleDateFormat,
                 java.util.Date" %>
<jsp:useBean id="smsStatFormBean" scope="page" class="ru.novosoft.smsc.jsp.smsstat.SmsStatFormBean"/>
<jsp:setProperty name="smsStatFormBean" property="*"/>
<%@ page contentType="text/csv; charset=windows-1251" %>
<%
    ru.novosoft.smsc.jsp.smsstat.SmsStatFormBean sbean = smsStatFormBean;
    int result = sbean.process(request);
    if (result == PageBean.RESULT_OK) {
        DateFormat dateFormat = new SimpleDateFormat("'stat'_yyyy-MM-dd_HH-mm-ss.'csv'");
        response.setHeader("Content-Disposition", "attachment; filename=" + dateFormat.format(new Date()));
        result = sbean.exportStatistics(request, out);
    }

    switch (result) {
        case PageBean.RESULT_ERROR:
        case SmsStatFormBean.RESULT_DLSTAT:
            response.sendRedirect("index.jsp?csv=true");
            return;
        default:
            break;
    }
%>