<%@include file="/common/header.jsp"%>

<%@page import="ru.novosoft.smsc.admin.smsview.*"%>

<jsp:useBean id="formBean" scope="session"
             class="ru.novosoft.smsc.jsp.smsview.SmsViewFormBean" />
<%
if (request.getMethod().equals("POST"))
{
%>
  <jsp:setProperty name="formBean" property="*" />
<%
  if (request.getParameter("prev") != null) {
    formBean.processPrev();
  } else if (request.getParameter("next") != null) {
    formBean.processNext();
  } else {
    if (formBean.getAppContext() == null) {
      formBean.setAppContext(request.getAttribute("appContext"));
    }
    formBean.processQuery();
  }
}
%>

<body>
<h1 align="center">SMSC SmsView utilite</h1>
<form action="smsview.jsp" method=POST>
<center>
<table cellpadding=4 cellspacing=2 border=0>
<tr>
    	<td valign=left>
    	<table cellpadding=4 cellspacing=2 border=0>
    	<tr>
    	<td valign=top>
    	<b>Source&nbsp;&nbsp;storage:&nbsp;&nbsp;</b>
    	  <input type="radio" name="storageType"
          value="<%= SmsQuery.SMS_ARCHIVE_STORAGE_TYPE%>"
          <%= (formBean.getStorageType()==SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) ?
              "checked":""%>>Archive&nbsp;&nbsp;
      	<input type="radio" name="storageType"
	        value="<%= SmsQuery.SMS_OPERATIVE_STORAGE_TYPE%>"
          <%= (formBean.getStorageType()==SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
              "checked":""%>>Operative&nbsp;&nbsp;
    	</td>
    	</tr>
    	<tr>
    	<td valign=top>
    	<b>From&nbsp;Address:&nbsp;&nbsp;</b>
    	<input type="text" name="fromAddress"
        value="<jsp:getProperty name="formBean" property="fromAddress" />" size=25></td>
    	<td valign=top>
    	<b>To&nbsp;Address:&nbsp;&nbsp;</b>
    	<input type="text" name="toAddress"
        value="<jsp:getProperty name="formBean" property="toAddress" />" size=25></td>
    	</tr>

    	<tr>
    	<td valign=top>
    	<b>From&nbsp;Date:&nbsp;&nbsp;</b>
    	<input type="text" name="fromDate" size=20
        value="<jsp:getProperty name="formBean" property="fromDate" />"></td>
    	<td valign=top>
    	<b>Till&nbsp;Date:&nbsp;&nbsp;</b>
    	<input type="text" name="tillDate" size=20
        value="<jsp:getProperty name="formBean" property="tillDate" />"></td>
    	</tr>
	</table>
	</td>

	<td valign=right>
	<table cellpadding=0 cellspacing=2 border=0>
	<tr><td><b>Sort&nbsp;by:</b>&nbsp;</td></tr>
	<tr><td>
	<select name="sortBy">
	    <option value="Date"
        <%= (formBean.getSortBy().equalsIgnoreCase("Date")) ? "selected":""%>>Date</option>
	    <option value="Status"
        <%= (formBean.getSortBy().equalsIgnoreCase("Status")) ? "selected":""%>>Status</option>
	    <option value="From"
        <%= (formBean.getSortBy().equalsIgnoreCase("From")) ? "selected":""%>>From</option>
	    <option value="To"
        <%= (formBean.getSortBy().equalsIgnoreCase("To")) ? "selected":""%>>To</option>
	</select>
	</td></tr>
	<tr><td><b>Rows&nbsp;to&nbsp;display:</b>&nbsp;</td></tr>
	<tr><td>
	<select name="rowsToDisplay">
      <% int rowsToDisplay = formBean.getRowsToDisplay();%>
	    <option value="5"
        <%= (rowsToDisplay < 10) ? "selected":""%>>5</option>
	    <option value="10"
        <%= (rowsToDisplay == 10) ? "selected":""%>>10</option>
	    <option value="20"
        <%= (rowsToDisplay == 20) ? "selected":""%>>20</option>
	    <option value="30"
        <%= (rowsToDisplay == 30) ? "selected":""%>>30</option>
	    <option value="40"
        <%= (rowsToDisplay == 40) ? "selected":""%>>40</option>
	    <option value="50"
        <%= (rowsToDisplay == 50) ? "selected":""%>>50</option>
	    <option value="-1"
        <%= (rowsToDisplay < 0 ||
             rowsToDisplay > 50) ? "selected":""%>>All</option>
	</select>
	</td></tr>
	</table>
	</td>
</tr>
<tr>
   	<td align=center colspan=2>
    	<input type="submit" name="query" value="Query !">
  	</td>
</tr>

<%
if (request.getMethod().equals("POST")) {
  int firstIndex = formBean.getRowIndex()+1;
  int lastIndex = formBean.getRowIndex()+formBean.getRowsToDisplay();
  if (lastIndex >= formBean.getRowsCount() ||
      formBean.getRowsToDisplay() < 0) lastIndex = formBean.getRowsCount();
%>
<tr>
    <td align=left colspan=2>
    <b>Total sms found </b><%= formBean.getRowsCount()%>
    <% if (formBean.getRowsCount()>0) { %>
        <b>, displayed </b><%= firstIndex%>
        <% if (firstIndex != lastIndex) {%>... <%= lastIndex%> <%};%>
    <% } else { %>
        <b>. Try to process another query. </b>
    <% } %>
    </td>
</tr>
<tr>
    <td align=center colspan=2>
    <% if (formBean.getRowsCount()>0) { %>
      <input type="submit" name="prev" value="<< Prev"
       <%= formBean.isPrevEnabled() ? "":"disabled"%>>
      <input type="submit" name="next" value="Next >>"
       <%= formBean.isNextEnabled() ? "":"disabled"%>>
    <% } %>
    </td>
</tr>
<tr>
    <td colspan=2>
    <!--table cellpadding=4 cellspacing=2 border=1-->
    <table width="100%" border=1>
      <tr>
      <td width="30%"><b>Date</b></td>
      <td width="10%"><b>From</b></td>
      <td width="10%"><b>To</b></td>
      <td width="10%"><b>Status</b></td>
      <td><b>Message</b></td>
      </tr>
      <% for (int cnt=firstIndex; cnt<=lastIndex; cnt++) {
          SmsRow row = formBean.getRow(cnt);
      %>
      <tr>
      <td><%= row.getDate()%></td>
      <td><%= row.getFrom()%></td>
      <td><%= row.getTo()%></td>
      <td><%= row.getStatus()%></td>
      <td><%= row.getText()+cnt%></td>
      </tr>
      <% } %>
    </table>
    </td>
</tr>
<tr>
    <td align=center colspan=2>
    <% if (formBean.getRowsCount()>0) { %>
      <input type="submit" name="prev" value="<< Prev"
       <%= formBean.isPrevEnabled() ? "":"disabled"%>>
      <input type="submit" name="next" value="Next >>"
       <%= formBean.isNextEnabled() ? "":"disabled"%>>
    <% } %>
    </td>
</tr>
<% } %>

</table>
</center>
</form>
</body>
<%@include file="/common/footer.jsp"%>
