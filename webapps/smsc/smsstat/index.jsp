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
int disNo = 1;
if (stat != null) {
    CountersSet total = stat.getTotal();
    Collection dates = stat.getDateStat();
%>
<script language="JavaScript">
  function toggleVisible(p, c)
  {
    var o = p.className == "collapsing_list_opened";
    p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
    c.runtimeStyle.display = o ? "none" : "block";
  }
</script>
<table class=list cellspacing=0>
<tr>
    <td colspan=7><div class=page_subtitle>General statistics</div></td>
</tr>
<tr class=row0>
    <th width="28%"><div align=right>&nbsp;</div></th>
    <th width="12%"><div align=right>Accepted</div></th>
    <th width="12%"><div align=right>Rejected</div></th>
    <th width="12%"><div align=right>Delivered</div></th>
    <th width="12%"><div align=right>Failed</div></th>
    <th width="12%"><div align=right>Rescheduled</div></th>
    <th width="12%"><div align=right>Temporal</div></th>
</tr>
<tr class=row1>
    <td width="28%" align=right>Total SMS processed:</td>
    <td width="12%" align=right><%= total.accepted%></td>
    <td width="12%" align=right><%= total.rejected%></td>
    <td width="12%" align=right><%= total.delivered%></td>
    <td width="12%" align=right><%= total.failed%></td>
    <td width="12%" align=right><%= total.rescheduled%></td>
    <td width="12%" align=right><%= total.temporal%></td>
</tr>
<%
Iterator i = dates.iterator();
while (i.hasNext()) {
    DateCountersSet date = (DateCountersSet)i.next();
    SimpleDateFormat formatter = new SimpleDateFormat("dd-MMM-yyyy");
    String dateStr = formatter.format(date.getDate());
%>  <tr class=row0>
        <td width="28%" align=right style="cursor:hand" onClick="toggleVisible(opForm.all.p<%=disNo%>, opForm.all.c<%=disNo%>);"><div id="p<%=disNo%>" class=collapsing_list_<%=i.hasNext() ? "closed" : "opened"%>><%= dateStr%><div></td>
        <td width="12%" align=right><%= date.accepted%></td>
        <td width="12%" align=right><%= date.rejected%></td>
        <td width="12%" align=right><%= date.delivered%></td>
        <td width="12%" align=right><%= date.failed%></td>
        <td width="12%" align=right><%= date.rescheduled%></td>
        <td width="12%" align=right><%= date.temporal%></td>
    </tr>
    <tr id="c<%=disNo++%>" style="display:<%=i.hasNext() ? "none" : "block"%>">
      <td colspan=7><table class=list cellspacing=0 cellpadding=0 border=0>
    <%
    Collection hours = date.getHourStat();
    Iterator j = hours.iterator();
    while (j.hasNext()) {
        HourCountersSet hour = (HourCountersSet)j.next();
    %>  <tr class=row1>
            <td width="28%" align=right>&nbsp;&nbsp;&nbsp;<%= ((hour.getHour() < 10)? "0"+hour.getHour():""+hour.getHour())%></td>
            <td width="12%" align=right><%= hour.accepted%></td>
            <td width="12%" align=right><%= hour.rejected%></td>
            <td width="12%" align=right><%= hour.delivered%></td>
            <td width="12%" align=right><%= hour.failed%></td>
            <td width="12%" align=right><%= hour.rescheduled%></td>
            <td width="12%" align=right><%= hour.temporal%></td>
        </tr><%
    }%>
    </table></td>
    </tr>
<%
}
    Collection smeids = stat.getSmeIdStat();
    i = smeids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=7><div class=page_subtitle>SME activity</div></td>
</tr>
<tr class=row0>
    <th width="28%"><div align=right>SME Id</div></th>
    <th width="12%"><div align=right>Accepted</div></th>
    <th width="12%"><div align=right>Rejected</div></th>
    <th width="12%"><div align=right>Delivered</div></th>
    <th width="12%"><div align=right>Failed</div></th>
    <th width="12%"><div align=right>Rescheduled</div></th>
    <th width="12%"><div align=right>Temporal</div></th>
</tr><%
        while (i.hasNext()) {
            SmeIdCountersSet sme = (SmeIdCountersSet)i.next();
            String smeStr = StringEncoderDecoder.encode(sme.smeid);
            Collection errs = sme.getErrors();
        %>
        <tr class=row0>
            <td width="28%" align=right style="cursor:hand" onClick="toggleVisible(opForm.all.p<%=disNo%>, opForm.all.c<%=disNo%>);"><div id="p<%=disNo%>" class=collapsing_list_closed><%= smeStr%><div></td>
            <td width="12%" align=right><%= sme.accepted%></td>
            <td width="12%" align=right><%= sme.rejected%></td>
            <td width="12%" align=right><%= sme.delivered%></td>
            <td width="12%" align=right><%= sme.failed%></td>
            <td width="12%" align=right><%= sme.rescheduled%></td>
            <td width="12%" align=right><%= sme.temporal%></td>
        </tr>
        <tr id="c<%=disNo++%>" style="display:none">
        <td colspan=7>
        <table class=list cellspacing=0 cellpadding=0 border=0>
        <%
          Iterator si = errs.iterator();
          while (si.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)si.next();
          %>
          <tr class=row1>
              <td width="28%" align=right nowrap>
          <%  String errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode."+errid.errcode);
              if (errMessage == null) errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode.unknown"); %>
          <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
          (<%=errid.errcode%>)
              </td>
              <td width="12%" align=right><%= errid.counter%></td>
              <td width="12%" align=right>&nbsp;</td>
              <td width="12%" align=right>&nbsp;</td>
              <td width="12%" align=right>&nbsp;</td>
              <td width="12%" align=right>&nbsp;</td>
              <td width="12%" align=right>&nbsp;</td>
          </tr><%
          }
        %>
        </table></td>
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
    <th width="28%"><div align=right>Route Id</div></th>
    <th width="12%"><div align=right>Accepted</div></th>
    <th width="12%"><div align=right>Rejected</div></th>
    <th width="12%"><div align=right>Delivered</div></th>
    <th width="12%"><div align=right>Failed</div></th>
    <th width="12%"><div align=right>Rescheduled</div></th>
    <th width="12%"><div align=right>Temporal</div></th>
</tr>  <%
        while (i.hasNext()) {
            RouteIdCountersSet route = (RouteIdCountersSet)i.next();
            String routeStr = StringEncoderDecoder.encode(route.routeid);
            Collection errs = route.getErrors();
        %>
        <tr class=row0>
            <td width="28%" align=right style="cursor:hand" onClick="toggleVisible(opForm.all.p<%=disNo%>, opForm.all.c<%=disNo%>);"><div id="p<%=disNo%>" class=collapsing_list_closed><%= routeStr%><div></td>
            <td width="12%" align=right><%= route.accepted%></td>
            <td width="12%" align=right><%= route.rejected%></td>
            <td width="12%" align=right><%= route.delivered%></td>
            <td width="12%" align=right><%= route.failed%></td>
            <td width="12%" align=right><%= route.rescheduled%></td>
            <td width="12%" align=right><%= route.temporal%></td>
        </tr>
        <tr id="c<%=disNo++%>" style="display:none">
        <td colspan=7>
        <table class=list cellspacing=0 cellpadding=0 border=0>
        <%
          Iterator ri = errs.iterator();
          while (ri.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)ri.next();
          %>
          <tr class=row1>
              <td width="28%" align=right nowrap>
          <%  String errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode."+errid.errcode);
              if (errMessage == null) errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode.unknown"); %>
          <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
          (<%=errid.errcode%>)
              </td>
              <td width="12%" align=right><%= errid.counter%></td>
              <td width="12%" align=right>&nbsp;</td>
              <td width="12%" align=right>&nbsp;</td>
              <td width="12%" align=right>&nbsp;</td>
              <td width="12%" align=right>&nbsp;</td>
              <td width="12%" align=right>&nbsp;</td>
          </tr><%
          }
        %>
        </table></td>
        </tr><%
        }
    }

    Collection errids = stat.getErrorStat();
    i = errids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=7><div class=page_subtitle>SMS delivery state</div></td>
</tr>
<tr class=row0>
    <th width="28%"><div align=right>Error Code</div></th>
    <th width="12%"><div align=right>Count</div></th>
    <th width="12%">&nbsp;</th>
    <th width="12%">&nbsp;</th>
    <th width="12%">&nbsp;</th>
    <th width="12%">&nbsp;</th>
    <th width="12%">&nbsp;</th>
</tr>  <%
        while (i.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)i.next();
        %>
        <tr class=row1>
            <td width="28%" align=right nowrap>
        <%
          String errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode."+errid.errcode);
          if (errMessage == null) errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode.unknown"); %>
        <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
        (<%=errid.errcode%>)
            </td>
            <td width="12%" align=right><%= errid.counter%></td>
            <td width="12%" align=right>&nbsp;</td>
            <td width="12%" align=right>&nbsp;</td>
            <td width="12%" align=right>&nbsp;</td>
            <td width="12%" align=right>&nbsp;</td>
            <td width="12%" align=right>&nbsp;</td>
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

