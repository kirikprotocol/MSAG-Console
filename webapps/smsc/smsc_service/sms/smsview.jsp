<%@include file="/common/header.jsp"%>

<jsp:useBean id="formBean" scope="session" class="ru.novosoft.smsc.jsp.smsview.SmsViewFormBean" />
<body>
<%
if (request.getMethod().equals("POST")) {
%>
  <jsp:setProperty name="formBean" property="*" />
<%
  if (request.getParameter("query") != null) {
    formBean.processQuery();
%>
  <h2>Query !</h2>
<%
  } else if (request.getParameter("prev") != null) {
    formBean.processPrev();
%>
  <h2>Prev !</h2>
<%
  } else if (request.getParameter("next") != null) {
    formBean.processNext();
%>
  <h2>Next !</h2>
<%
  }
}
%>

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
          value="<%= ru.novosoft.smsc.admin.smsview.SmsQuery.SMS_ARCHIVE_STORAGE_TYPE%>"
          <%= (formBean.getStorageType() ==
              ru.novosoft.smsc.admin.smsview.SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) ?
              "checked":""%>>Archive&nbsp;&nbsp;
      	<input type="radio" name="storageType"
	        value="<%= ru.novosoft.smsc.admin.smsview.SmsQuery.SMS_OPERATIVE_STORAGE_TYPE%>"
          <%= (formBean.getStorageType() ==
              ru.novosoft.smsc.admin.smsview.SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
              "checked":""%>>Operative&nbsp;&nbsp;
    	</td>
    	</tr>
    	<tr>
    	<td valign=top>
    	<b>From&nbsp;Address:&nbsp;&nbsp;</b>
    	<input type="text" name="fromAddress"
        value="<jsp:getProperty name="formBean" property="fromAddress" />" size=20></td>
    	<td valign=top>
    	<b>To&nbsp;Address:&nbsp;&nbsp;</b>
    	<input type="text" name="toAddress"
        value="<jsp:getProperty name="formBean" property="toAddress" />" size=20></td>
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
	<tr><td><b>Show&nbsp;results:</b>&nbsp;</td></tr>
	<tr><td>
	<select name="rowsCount">
	    <option value="10"
        <%= (formBean.getRowsCount() == 10) ? "selected":""%>>10</option>
	    <option value="20"
        <%= (formBean.getRowsCount() == 20) ? "selected":""%>>20</option>
	    <option value="30"
        <%= (formBean.getRowsCount() == 30) ? "selected":""%>>30</option>
	    <option value="40"
        <%= (formBean.getRowsCount() == 40) ? "selected":""%>>40</option>
	    <option value="50"
        <%= (formBean.getRowsCount() == 50) ? "selected":""%>>50</option>
	    <option value="-1"
        <%= (formBean.getRowsCount() < 0) ? "selected":""%>>All</option>
	</select>
	</td></tr>
	</table>
	</td>
</tr>
<tr>
   	<td align=center colspan=2>
    	<input type="submit" name="query" value="Query !">
	    <!--input type="reset" name="reset" value="Reset"-->
  	</td>
</tr>

<%
if (request.getMethod().equals("POST")) {
%>
<tr>
    <td align=left colspan=2>
    <b>Total sms found </b>
    <%= formBean.getRowsTotal()%>
    <b>, displayed </b>
    <%= formBean.getRowIndex()%>-<%=
        ((formBean.getRowIndex()+formBean.getRowsCount()) >= formBean.getRowsTotal()
         || formBean.getRowsCount() < 0) ?
            formBean.getRowsTotal() :
              formBean.getRowIndex()+formBean.getRowsCount()%>
    </td>
</tr>
<tr>
    <td colspan=2>
    <!--table cellpadding=4 cellspacing=2 border=1-->
    <table width="100%" border=1>
      <tr>
      <td width="10%"><b>From</b></td>
      <td width="10%"><b>To</b></td>
      <td width="10%"><b>Date</b></td>
      <td width="10%"><b>Status</b></td>
      <td><b>Message</b></td>
      </tr>

      <tr>
      <td>From1</td>
      <td>To1</td>
      <td>Date1</td>
      <td>Status1</td>
      <td>Text1</td>
      </tr>
    </table>
    </td>
</tr>
<tr>
    <td align=center colspan=2>
      <%if (formBean.isPrevEnabled()) {%>
        <input type="submit" name="prev" value="<< Prev">
      <%}%>
      <%if (formBean.isNextEnabled()) {%>
        <input type="submit" name="next" value="Next >>">
      <%}%>
    </td>
</tr>
<%
}
%>

</table>
</center>
</form>
</body>
<%@include file="/common/footer.jsp"%>
