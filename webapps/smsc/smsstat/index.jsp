<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, java.text.SimpleDateFormat"%>
<%@ page import="ru.novosoft.smsc.admin.smsstat.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsstat.*"%>
<jsp:useBean id="bean" scope="session" class="ru.novosoft.smsc.jsp.smsstat.SmsStatFormBean" />
<%
    bean.setFromDate(null);
    bean.setTillDate(null);
%>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE="SMSC Statistics utilite";
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

<div class=secQuestion>Statistics parameters</div>
<table class=secRep width="100%">
<tr class=row0>
	<th class=label>From Date:</th>
	<td nowrap><input type=text id="fromDate" name="fromDate" class=calendarField value="<%=bean.getFromDate()%>" maxlength=19 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
	<th class=label>Till Date:</th>
	<td nowrap><input type=text id="tillDate" name="tillDate" class=calendarField value="<%=bean.getTillDate()%>" maxlength=19><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
</table>
<div class=secButtons>
    <input class=btn type="submit" name="mbQuery" value="Query !">
</div>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
Statistics stat = bean.getStatistics();
CountersSet total = stat.getTotal();
Collection dates = stat.getDateStat();
if (stat != null) {
%>
<div class=secView>General statistics</div>
<table class=secRep cellspacing=1 width="100%">
    <th></th>
    <th>Accepted</th>
    <th>Finalized</th>
    <th>Rescheduled</th>
<tr class=row1>
    <td><b>Total:</b></td>
    <td><%= total.accepted%></td>
    <td><%= total.finalized%></td>
    <td><%= total.rescheduled%></td>
</tr>
<%
Iterator i = dates.iterator();
while (i.hasNext()) {
    DateCountersSet date = (DateCountersSet)i.next();
    int fromHour = date.getStartHour();
    SimpleDateFormat formatter = new SimpleDateFormat("dd-MMMM-yyyy");
%>  <tr class=row0>
        <td><%= formatter.format(date.getDate())%></td>
        <td><%= date.accepted%></td>
        <td><%= date.finalized%></td>
        <td><%= date.rescheduled%></td>
    </tr><%
    Collection hours = date.getHourStat();
    Iterator j = hours.iterator();
    while (j.hasNext()) {
        CountersSet hour = (CountersSet)j.next();
    %>  <tr class=row1>
            <td><%= fromHour++%></td>
            <td><%= hour.accepted%></td>
            <td><%= hour.finalized%></td>
            <td><%= hour.rescheduled%></td>
        </tr>
    <%}%>
<%}%>
</table>
<%
    Collection smeids = stat.getSmeIdStat();
    i = smeids.iterator();
    if (i.hasNext()) {
%>
<div class=secView>SME activity statistics</div>
<table class=secRep cellspacing=1 width="100%">
    <th>SME Id</th>
    <th>Sent</th>
    <th>Received</th><%
        while (i.hasNext()) {
            SmeIdCountersSet smeid = (SmeIdCountersSet)i.next();
        %>
        <tr class=row1>
            <td><%= smeid.smeid%></td>
            <td><%= smeid.sent%></td>
            <td><%= smeid.received%></td>
        </tr><%
        }%>
</table>
<% }
    Collection routeids = stat.getRouteIdStat();
    i = routeids.iterator();
    if (i.hasNext()) {
%>
<div class=secView>Route traffic statistics</div>
<table class=secRep cellspacing=1 width="100%">
    <th>Route Id</th>
    <th>Processed</th><%
        while (i.hasNext()) {
            RouteIdCountersSet routeid = (RouteIdCountersSet)i.next();
        %>
        <tr class=row1>
            <td><%= routeid.routeid%></td>
            <td><%= routeid.counter%></td>
        </tr><%
        }%>
</table>
<% }
    Collection errids = stat.getErrorStat();
    i = errids.iterator();
    if (i.hasNext()) {
%>
<div class=secView>SMS Status statistics</div>
<table class=secRep cellspacing=1 width="100%">
    <th>Error Code</th>
    <th>Count</th><%
        while (i.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)i.next();
        %>
        <tr class=row1>
            <td><%= errid.errcode%></td>
            <td><%= errid.counter%></td>
        </tr><%
        }%>
</table>
<% }
} // stat != null%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

