<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, java.text.SimpleDateFormat,
                 ru.novosoft.smsc.jsp.SMSCAppContext,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
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
    switch(beanResult = bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
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
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>

<div class=secQuestion>Query statistics</div>
<table class=secRep width="100%">
<tr class=row0>
	<th class=label>From Date:</th>
	<td nowrap><input type=text id="fromDate" name="fromDate" class=calendarField value="<%=bean.getFromDate()%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
	<th class=label>Till Date:</th>
	<td nowrap><input type=text id="tillDate" name="tillDate" class=calendarField value="<%=bean.getTillDate()%>" maxlength=20><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
</table>
<div class=secButtons>
    <input class=btn type="submit" name="mbQuery" value="Query !">
</div>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
Statistics stat = bean.getStatistics();
if (stat != null) {
    CountersSet total = stat.getTotal();
    Collection dates = stat.getDateStat();
%>
<table class=secRep cellspacing=1 width="100%">
<tr>
    <td colspan=4><div class=secView>General statistics</div></td>
</tr>
<tr class=row0>
    <th class=label width="25%">&nbsp;</th>
    <th width="25%"><div align=right>Accepted</div></th>
    <th width="25%"><div align=right>Finalized</div></th>
    <th width="25%"><div align=right>Rescheduled</div></th>
</tr>
<tr class=row1>
    <th class=label>Total SMS processed:</th>
    <td align=right><%= total.accepted%></td>
    <td align=right><%= total.finalized%></td>
    <td align=right><%= total.rescheduled%></td>
</tr>
<%
Iterator i = dates.iterator();
while (i.hasNext()) {
    DateCountersSet date = (DateCountersSet)i.next();
    SimpleDateFormat formatter = new SimpleDateFormat("dd-MMM-yyyy");
%>  <tr class=row0>
        <th class=label><%= formatter.format(date.getDate())%></th>
        <td align=right><%= date.accepted%></td>
        <td align=right><%= date.finalized%></td>
        <td align=right><%= date.rescheduled%></td>
    </tr><%
    Collection hours = date.getHourStat();
    Iterator j = hours.iterator();
    while (j.hasNext()) {
        HourCountersSet hour = (HourCountersSet)j.next();
    %>  <tr class=row1>
            <th class=label>&nbsp;&nbsp;&nbsp;<%= ((hour.getHour() < 10)? "0"+hour.getHour():""+hour.getHour())%></th>
            <td align=right><%= hour.accepted%></td>
            <td align=right><%= hour.finalized%></td>
            <td align=right><%= hour.rescheduled%></td>
        </tr>
  <%}
}
    Collection smeids = stat.getSmeIdStat();
    i = smeids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=4><div class=secView>SME activity</div></td>
</tr>
<tr class=row0>
    <th width="25%"><div align=right>SME Id</div></th>
    <th width="25%"><div align=right>Sent</div></th>
    <th width="25%"><div align=right>Received</div></th>
    <th class=label width="25%">&nbsp;</th>
</tr><%
        while (i.hasNext()) {
            SmeIdCountersSet smeid = (SmeIdCountersSet)i.next();
        %>
        <tr class=row1>
            <td align=right><%= StringEncoderDecoder.encode(smeid.smeid)%></td>
            <td align=right><%= smeid.sent%></td>
            <td align=right><%= smeid.received%></td>
            <td align=right>&nbsp;</td>
        </tr><%
        }
    }
    Collection routeids = stat.getRouteIdStat();
    i = routeids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=4> <div class=secView>Traffic by routes</div></td>
</tr>
<tr class=row0>
    <th width="25%"><div align=right>Route Id</div></th>
    <th width="25%"><div align=right>Processed</div></th>
    <th class=label width="25%">&nbsp;</th>
    <th class=label width="25%">&nbsp;</th>
</tr>  <%
        while (i.hasNext()) {
            RouteIdCountersSet routeid = (RouteIdCountersSet)i.next();
        %>
        <tr class=row1>
            <td align=right><%= StringEncoderDecoder.encode(routeid.routeid)%></td>
            <td align=right><%= routeid.counter%></td>
            <td align=right>&nbsp;</td>
            <td align=right>&nbsp;</td>
        </tr><%
        }
    }
    Collection errids = stat.getErrorStat();
    i = errids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=4><div class=secView>SMS delivery state</div></td>
</tr>
<tr class=row0>
    <th width="25%"><div align=right>Error Code</div></th>
    <th width="25%"><div align=right>Count</div></th>
    <th class=label width="25%">&nbsp;</th>
    <th class=label width="25%">&nbsp;</th>
</tr>  <%
        while (i.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)i.next();
        %>
        <tr class=row1>
            <td align=right nowrap>
        <%
          String errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode."+errid.errcode);
          if (errMessage == null) errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode.unknown"); %>
        <%= StringEncoderDecoder.encode(errMessage)%>
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

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

