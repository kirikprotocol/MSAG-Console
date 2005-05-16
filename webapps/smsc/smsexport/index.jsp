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
      response.sendRedirect("index.jsp");
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
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content><%
{%>
<div class=page_subtitle>Database Settings</div>
<table class=properties_list cell>
<tr class=row0>
  <th>Data Source:</th>
  <td><input class=txt type="text" id="source" name="source" value="<%=defExpSett.getSource()%>" size=25 maxlength=25 onkeyup="hideAddresses()"></td>
  <th>Driver:</th>
  <td><input class=txt type="text" id="driver" name="driver" value="<%=defExpSett.getDriver()%>" size=17 maxlength=15 onkeyup="hideSmeIds()"></td>
</tr>
<tr class=row1>
  <th>User:</th>
  <td><input class=txt type="text" id="user" name="user"  value="<%=defExpSett.getUser()%>" size=25 maxlength=25></td>
  <th>Password:</th>
  <td><input class=txt type="text" id="password" name="password"  value="<%=defExpSett.getPassword()%>" size=17 maxlength=15></td>
</tr>
<tr class=row0>
  <th>Table Name:</th>
  <td><input class=txt type="text" id="tablesPrefix" name="tablesPrefix" value="<%=defExpSett.getTablesPrefix()%>" size=25 maxlength=25></td>
  <th>&nbsp</th>
  <td>&nbsp</td>
</tr>
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