<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, java.text.SimpleDateFormat,
                 ru.novosoft.smsc.jsp.SMSCAppContext,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.SMSCJspException"%>
<%@ page import="ru.novosoft.smsc.admin.smsstat.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsstat.*"%>
<jsp:useBean id="smsStatFormBean" scope="session" class="ru.novosoft.smsc.jsp.smsstat.SmsStatFormBean" />
<%
	SmsStatFormBean bean = smsStatFormBean;
    bean.setFromDate(null);
    bean.setTillDate(null);
%>
<jsp:setProperty name="smsStatFormBean" property="*"/>
<%
    TITLE="SMS Statistics";
    MENU0_SELECTION = "MENU0_SMSSTAT";

    int beanResult = SmsStatFormBean.RESULT_OK;
    switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
    {
        case SmsStatFormBean.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case SmsStatFormBean.RESULT_FILTER:
        case SmsStatFormBean.RESULT_OK:
            STATUS.append("Ok");
            break;
        case SmsStatFormBean.RESULT_ERROR:
            STATUS.append("<span class=CF00>Error</span>");
            break;
        default:
            STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<div class=content>

<div class=page_subtitle>Query statistics</div>
<table class=properties_list cellspacing=0>
<tr class=row0>
	<th>From Date:</th>
	<td nowrap><input type=text id="fromDate" name="fromDate" class=calendarField value="<%=bean.getFromDate()%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
	<th>Till Date:</th>
	<td nowrap><input type=text id="tillDate" name="tillDate" class=calendarField value="<%=bean.getTillDate()%>" maxlength=20><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbQuery",  "Query !",  "Run query");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
Statistics stat = bean.getStatistics();
if (stat != null) {
    CountersSet total = stat.getTotal();
    Collection dates = stat.getDateStat();
%>
<table class=list cellspacing=0>
<tr>
    <td colspan=7><div class=page_subtitle>General statistics</div></td>
</tr>
<tr class=row0>
    <th width="22%">&nbsp;</th>
    <th width="13%"><div align=right>Accepted</div></th>
    <th width="13%"><div align=right>Rejected</div></th>
    <th width="13%"><div align=right>Delivered</div></th>
    <th width="13%"><div align=right>Failed</div></th>
    <th width="13%"><div align=right>Rescheduled</div></th>
    <th width="13%"><div align=right>Temporal</div></th>
</tr>
<tr class=row1>
    <td>Total SMS processed:</td>
    <td align=right><%= total.accepted%></td>
    <td align=right><%= total.rejected%></td>
    <td align=right><%= total.delivered%></td>
    <td align=right><%= total.failed%></td>
    <td align=right><%= total.rescheduled%></td>
    <td align=right><%= total.temporal%></td>
</tr>
<%
Iterator i = dates.iterator();
while (i.hasNext()) {
    DateCountersSet date = (DateCountersSet)i.next();
    SimpleDateFormat formatter = new SimpleDateFormat("dd-MMM-yyyy");
%>  <tr class=row0>
        <td><%= formatter.format(date.getDate())%></td>
        <td align=right><%= date.accepted%></td>
        <td align=right><%= date.rejected%></td>
        <td align=right><%= date.delivered%></td>
        <td align=right><%= date.failed%></td>
        <td align=right><%= date.rescheduled%></td>
        <td align=right><%= date.temporal%></td>
    </tr><%
    Collection hours = date.getHourStat();
    Iterator j = hours.iterator();
    while (j.hasNext()) {
        HourCountersSet hour = (HourCountersSet)j.next();
    %>  <tr class=row1>
            <td>&nbsp;&nbsp;&nbsp;<%= ((hour.getHour() < 10)? "0"+hour.getHour():""+hour.getHour())%></td>
            <td align=right><%= hour.accepted%></td>
            <td align=right><%= hour.rejected%></td>
            <td align=right><%= hour.delivered%></td>
            <td align=right><%= hour.failed%></td>
            <td align=right><%= hour.rescheduled%></td>
            <td align=right><%= hour.temporal%></td>
        </tr>
  <%}
}
    Collection smeids = stat.getSmeIdStat();
    i = smeids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=7><div class=page_subtitle>SME activity</div></td>
</tr>
<tr class=row0>
    <th width="22%">SME Id</th>
    <th width="13%"><div align=right>Accepted</div></th>
    <th width="13%"><div align=right>Rejected</div></th>
    <th width="13%"><div align=right>Delivered</div></th>
    <th width="13%"><div align=right>Failed</div></th>
    <th width="13%"><div align=right>Rescheduled</div></th>
    <th width="13%"><div align=right>Temporal</div></th>
</tr><%
        while (i.hasNext()) {
            SmeIdCountersSet sme = (SmeIdCountersSet)i.next();
        %>
        <tr class=row1>
            <td align=right><%= StringEncoderDecoder.encode(sme.smeid)%></td>
            <td align=right><%= sme.accepted%></td>
            <td align=right><%= sme.rejected%></td>
            <td align=right><%= sme.delivered%></td>
            <td align=right><%= sme.failed%></td>
            <td align=right><%= sme.rescheduled%></td>
            <td align=right><%= sme.temporal%></td>
        </tr><%
        }
    }
    Collection routeids = stat.getRouteIdStat();
    i = routeids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=7> <div class=page_subtitle>Traffic by routes</div></td>
</tr>
<tr class=row0>
    <th width="22%"><div align=right>Route Id</div></th>
    <th width="13%"><div align=right>Accepted</div></th>
    <th width="13%"><div align=right>Rejected</div></th>
    <th width="13%"><div align=right>Delivered</div></th>
    <th width="13%"><div align=right>Failed</div></th>
    <th width="13%"><div align=right>Rescheduled</div></th>
    <th width="13%"><div align=right>Temporal</div></th>
</tr>  <%
        while (i.hasNext()) {
            RouteIdCountersSet route = (RouteIdCountersSet)i.next();
        %>
        <tr class=row1>
            <td align=right><%= StringEncoderDecoder.encode(route.routeid)%></td>
            <td align=right><%= route.accepted%></td>
            <td align=right><%= route.rejected%></td>
            <td align=right><%= route.delivered%></td>
            <td align=right><%= route.failed%></td>
            <td align=right><%= route.rescheduled%></td>
            <td align=right><%= route.temporal%></td>
        </tr><%
        }
    }
    Collection errids = stat.getErrorStat();
    i = errids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=4><div class=page_subtitle>SMS delivery state</div></td>
</tr>
<tr class=row0>
    <th width="25%"><div align=right>Error Code</div></th>
    <th width="25%"><div align=right>Count</div></th>
    <th width="25%">&nbsp;</th>
    <th width="25%">&nbsp;</th>
</tr>  <%
        while (i.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)i.next();
        %>
        <tr class=row1>
            <td align=right nowrap>
        <%
          String errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode."+errid.errcode);
          if (errMessage == null) errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode.unknown"); %>
        <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
        (<%=errid.errcode%>)
            </td>
            <td align=right><%= errid.counter%></td>
            <td align=right>&nbsp;</td>
            <td align=right>&nbsp;</td>
        </tr><%
        }
    }
%>
</table>
<%
} // stat != null%>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

