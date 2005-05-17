<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.smsexport.*,
                 ru.novosoft.smsc.util.*,
                 java.text.*,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.Collection,
                 java.util.Iterator,
                 ru.novosoft.smsc.admin.smsview.SmsQuery,
                 ru.novosoft.smsc.admin.smsview.SmsRow,
                 ru.novosoft.smsc.admin.smsstat.ExportSettings"%>
<%@ page import="ru.novosoft.smsc.jsp.smsexport.*"%>
<jsp:useBean id="smsExportBean" scope="session" class="ru.novosoft.smsc.jsp.smsexport.SmsExportBean" />
<%
  SmsExportBean bean = smsExportBean;

%>
<jsp:setProperty name="smsExportBean" property="*"/>
<%
  TITLE="SMS Export";
  MENU0_SELECTION = "MENU0_SMSEXPORT";

  int beanResult = bean.process(request);
  switch(beanResult)
  {
    case SmsExportBean.RESULT_DONE:
      response.sendRedirect("export.jsp");
      return;
    case SmsExportBean.RESULT_FILTER:
    case SmsExportBean.RESULT_OK:
      break;
    case SmsExportBean.RESULT_ERROR:
      break;
    default:
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
  ExportSettings defExpSett=bean.getDefaultExportSettings();
int rowN=0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content><%
{%>
<script>
function default(){
  document.opForm.inp.value="defaultval";
}
function disable(){
  document.opForm.inp.disabled=true;
}

</script>
<div class=page_subtitle>Database Settings</div>
<table class=properties_list cell>
<tr class=row<%= (rowN++)%2%>>
  <!-- TODO: add script feature to disable/restore edit fields when switching to DEFAULT destination -->
  <td nowrap><input
      class=radio type="radio" name="destination" id=destinationDefault value="<%= SmsExportBean.DEFAULT_DESTINATION%>"
      <%= (bean.getDestination() == SmsExportBean.DEFAULT_DESTINATION) ? "checked":""%>>
      <label for=destinationDefault>Default destination</label></td>
  <td nowrap><input
      class=radio type="radio" name="destination" id=destinationUser value="<%= SmsExportBean.USER_DESTINATION%>"
      <%= (bean.getDestination() == SmsExportBean.USER_DESTINATION) ? "checked":""%>>
      <label for=destinationUser>User defined</label>
  </td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>JDBC source</th>
	<td nowrap><input class=txt type=text id="source" name="source" value="<%=defExpSett.getSource()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>JDBC driver</th>
	<td nowrap><input class=txt type=text id="driver" name="driver" value="<%=defExpSett.getDriver()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>User</th>
	<td nowrap><input class=txt type=text id="user" name="user" value="<%=defExpSett.getUser()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Password</th>
	<td nowrap><input class=txt type=password id="password" name="password" value="<%=defExpSett.getPassword()%>"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
	<th>Table Name:</th>
	<td nowrap><input class=txt type=text id="tablesPrefix" name="tablesPrefix" value="<%=defExpSett.getTablesPrefix()%>"></td>
</tr>

<%--
<tr class=row<%= (rowN++)%2%>>
  <th>Data Source:</th>
  <td><input class=txt type="text" id="source" name="source" value="<%=defExpSett.getSource()%>" size=25 maxlength=25 onkeyup="hideAddresses()"></td>
  <th>Driver:</th>
  <td><input class=txt type="text" id="driver" name="driver" value="<%=defExpSett.getDriver()%>" size=17 maxlength=15 onkeyup="hideSmeIds()"></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
  <th>User:</th>
  <td><input class=txt type="text" id="user" name="user"  value="<%=defExpSett.getUser()%>" size=25 maxlength=25></td>
  <th>Password:</th>
  <td><input class=txt type="text" id="password" name="password"  value="<%=defExpSett.getPassword()%>" size=17 maxlength=15></td>
</tr>
<tr class=row<%= (rowN++)%2%>>
  <th>Table Name:</th>
  <td><input class=txt type="text" id="tablesPrefix" name="tablesPrefix" value="<%=defExpSett.getTablesPrefix()%>" size=25 maxlength=25></td>
  <th>&nbsp</th>
  <td>&nbsp</td>
</tr>  --%>
</table>
<%}
%></div><%
page_menu_begin(out);
page_menu_button(out, "mbExport",  "Export !",  "Run export");
page_menu_space(out);
page_menu_end(out);
%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>