<%@ include file="/common/header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.smsview.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsview.*"%>

<jsp:useBean id="formBean" scope="session"
             class="ru.novosoft.smsc.jsp.smsview.SmsViewFormBean" />
<%
if (request.getParameter("prev") != null) {
    formBean.moveToPrev();
} else if (request.getParameter("next") != null) {
    formBean.moveToNext();
} else if (request.getParameter("first") != null) {
    formBean.moveToFirst();
} else if (request.getParameter("last") != null) {
    formBean.moveToLast();
}

String pageCountString = (String)request.getParameter("page");
if (pageCountString != null) formBean.moveToPage(pageCountString);

if (request.getMethod().equals("POST"))
{
%>
  <jsp:setProperty name="formBean" property="*" />
<%
  String fde = (String)request.getParameter("fromDateEnabled");
  formBean.setFromDateEnabled(fde != null && fde.equalsIgnoreCase("on"));
  String tde = (String)request.getParameter("tillDateEnabled");
  formBean.setTillDateEnabled(tde != null && tde.equalsIgnoreCase("on"));
  if (formBean.getAppContext() == null) {
    formBean.setAppContext(request.getAttribute("appContext"));
  }
  formBean.processQuery();
}
%>
<body>
<h1 align="center">SMSC SmsView utilite</h1>
<form action="smsview.jsp" method=POST>
<center>
<table width="95%" cellpadding=2 cellspacing=2 border=0>
<tr>
    	<td colspan=3 valign=top>
    	<b>Source&nbsp;&nbsp;storage:&nbsp;&nbsp;</b>
    	<input type="radio" name="storageType" value="<%=
        SmsQuery.SMS_ARCHIVE_STORAGE_TYPE%>" <%=
          (formBean.getStorageType()==SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) ?
              "checked":""%>>Archive&nbsp;&nbsp;
      <input type="radio" name="storageType" value="<%=
        SmsQuery.SMS_OPERATIVE_STORAGE_TYPE%>" <%=
          (formBean.getStorageType()==SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
              "checked":""%>>Operative&nbsp;&nbsp;
    	</td>
</tr>
<tr>
    	<td colspan=1>
      <b>Source&nbsp;address:</b><br>
    	<input type="text" name="fromAddress"  value="<%=
        formBean.getFromAddress()%>" size=25 maxlength=25>
      </td>
      <td colspan=1>
      <b>Source&nbsp;SME&nbsp;id:</b><br>
    	<input type="text" name="srcSmeId"  value="<%=
        formBean.getSrcSmeId()%>" size=15 maxlength=15>
      </td>
      <td colspan=1>
    	<b>Select&nbsp;from&nbsp;date:</b><br>
      <input type="checkbox" name="fromDateEnabled" <%=
        (formBean.getFromDateEnabled()) ? "checked":""%>>
      <input type="text" name="fromDateDay" style="width:16pt;" value="<%=
        formBean.getFromDateDay()%>" size=2 maxlength=2>
      <select name="fromDateMonth">
      <%int fromDateMonth = formBean.getFromDateMonth();
      for (int fdm=0; fdm<12; fdm++) {
      %><option value="<%= fdm%>" <%=
          (fdm == fromDateMonth) ? "selected":""%>><%=
            SmsViewFormBean.monthesNames[fdm] %></option><%
      }%>
	    </select>
      <select name="fromDateYear">
      <% int fromDateYear = formBean.getFromDateYear();
      for (int fdy=SmsViewFormBean.START_YEAR_COUNTER;
               fdy<=SmsViewFormBean.FINISH_YEAR_COUNTER; fdy++) {
      %><option value="<%= fdy%>" <%=
          (fdy == fromDateYear) ? "selected":""%>><%= fdy%></option><%
      }%>
      </select>&nbsp;
      <input type="text" name="fromDateHour" style="width:16pt;" value="<%=
        formBean.getFromDateHour()%>" size=2 maxlength=2>
      <b>:</b>
      <input type="text" name="fromDateMinute" style="width:16pt;" value="<%=
        formBean.getFromDateMinute()%>" size=2 maxlength=2>
      <b>:</b>
      <input type="text" name="fromDateSecond" style="width:16pt;" value="<%=
        formBean.getFromDateSecond()%>" size=2 maxlength=2>
      </td>
</tr>
<tr>
      <td colspan=1>
    	<b>Destination&nbsp;address:</b><br>
    	<input type="text" name="toAddress" value="<%=
        formBean.getToAddress()%>" size=25 maxlength=25>
      </td>
      <td colspan=1>
      <b>Destination&nbsp;SME&nbsp;id:</b><br>
    	<input type="text" name="dstSmeId"  value="<%=
        formBean.getDstSmeId()%>" size=15 maxlength=15>
      </td>
      <td colspan=1>
    	<b>Till&nbsp;date:</b><br>
      <input type="checkbox" name="tillDateEnabled" <%=
        (formBean.getTillDateEnabled()) ? "checked":""%>>
      <input type="text" name="toDateDay" style="width:16pt;" value="<%=
        formBean.getToDateDay()%>" size=2 maxlength=2>
      <select name="toDateMonth">
      <% int toDateMonth = formBean.getToDateMonth();
      for (int fdm=0; fdm<12; fdm++) {
      %><option value="<%= fdm%>" <%=
        (fdm == toDateMonth) ? "selected":""%>><%=
          SmsViewFormBean.monthesNames[fdm]%></option><%
      }%>
      </select>
      <select name="toDateYear">
      <% int toDateYear = formBean.getToDateYear();
      for (int tdy=SmsViewFormBean.START_YEAR_COUNTER;
               tdy<=SmsViewFormBean.FINISH_YEAR_COUNTER; tdy++) {
      %><option value="<%= tdy%>" <%=
          (tdy == toDateYear) ? "selected":""%>><%= tdy%></option><%
      }%>
	    </select>&nbsp;
      <input type="text" name="toDateHour" style="width:16pt;" value="<%=
        formBean.getToDateHour()%>" size=2 maxlength=2>
      <b>:</b>
      <input type="text" name="toDateMinute" style="width:16pt;" value="<%=
        formBean.getToDateMinute()%>" size=2 maxlength=2>
      <b>:</b>
      <input type="text" name="toDateSecond" style="width:16pt;" value="<%=
        formBean.getToDateSecond()%>" size=2 maxlength=2>
      </td>
</tr>
<tr>
      <td colspan=1>
      <b>Route&nbsp;id:</b><br>
    	<input type="text" name="routeId"  value="<%=
        formBean.getRouteId()%>" size=20 maxlength=20>
      </td>
      <td colspan=1>
      <b>Rows&nbsp;per&nbsp;page:</b><br>
    	<select name="rowsToDisplay"><%
        int rowsToDisplay = formBean.getRowsToDisplay();%>
	      <option value="5" <%= (rowsToDisplay < 10) ?   "selected":""%>>5</option>
	      <option value="10" <%= (rowsToDisplay == 10) ? "selected":""%>>10</option>
	      <option value="20" <%= (rowsToDisplay == 20) ? "selected":""%>>20</option>
	      <option value="30" <%= (rowsToDisplay == 30) ? "selected":""%>>30</option>
	      <option value="40" <%= (rowsToDisplay == 40) ? "selected":""%>>40</option>
	      <option value="50" <%= (rowsToDisplay == 50) ? "selected":""%>>50</option>
	      <option value="-1" <%= (rowsToDisplay < 0 ||
                                rowsToDisplay > 50) ?  "selected":""%>>All</option>
	    </select>
      </td>
      <td colspan=1>
    	<b>Sort&nbsp;results&nbsp;by:</b><br>
      <select name="sortBy">
	      <option value="Date" <%=
          (formBean.getSortBy().equalsIgnoreCase("Date")) ? "selected":""%>>Date</option>
	      <option value="Status" <%=
          (formBean.getSortBy().equalsIgnoreCase("Status")) ? "selected":""%>>Status</option>
	      <option value="From" <%=
          (formBean.getSortBy().equalsIgnoreCase("From")) ? "selected":""%>>From</option>
	      <option value="To" <%=
          (formBean.getSortBy().equalsIgnoreCase("To")) ? "selected":""%>>To</option>
      </select>
	    </td>
</tr>
<tr>
  <td colspan=3 align=center>
    <input type="submit" name="query" value="Query !">
  </td>
</tr>
</table>

<%
int firstIndex = formBean.getRowIndex()+1;
int lastIndex = formBean.getRowIndex()+formBean.getRowsToDisplay();
if (lastIndex >= formBean.getRowsCount() ||
    formBean.getRowsToDisplay() < 0) lastIndex = formBean.getRowsCount();
%>
<%@include file="smsindex.jsp"%>
<%if (formBean.getRowsCount()>0) {%>
<table width="95%" cellpadding=1 cellspacing=1 border=1>
<tr>
  <td width="25%"><b>Date</b></td>
  <td width="10%"><b>From</b></td>
  <td width="10%"><b>To</b></td>
  <td width="10%"><b>Status</b></td>
  <td><b>Message</b></td>
</tr><%
for (int cnt=firstIndex; cnt<=lastIndex; cnt++) {
  SmsRow row = formBean.getRow(cnt-1);
%><tr>
      <td><%= row.getDate()%></td>
      <td><%= row.getFrom()%></td><td><%= row.getTo()%></td>
      <td><%= row.getStatus()%></td>
      <td><%= "#"+cnt+" "+row.getText()%></td>
  </tr><%
} %>
</table>
<%@include file="smsindex.jsp"%>
<% } %>
</center>
</form>
</body>

<%@include file="/common/footer.jsp"%>