<%@
 include file="/WEB-INF/inc/code_header.jsp"%><%@
 page import="ru.novosoft.smsc.admin.service.ServiceInfo,
              ru.novosoft.smsc.admin.Constants,
              java.io.IOException,
              ru.novosoft.smsc.util.StringEncoderDecoder,
              java.util.Iterator,
              ru.novosoft.smsc.jsp.SMSCJspException,
              ru.novosoft.smsc.jsp.SMSCErrors,
              ru.novosoft.smsc.jsp.smsc.reshedule.Index,
              java.util.Collection,
              ru.novosoft.smsc.jsp.SMSCAppContext,
              java.util.Locale,
              java.net.URLEncoder"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.reshedule.Index"
/><jsp:setProperty name="bean" property="*"/><%
//ServiceIDForShowStatus = Constants.SMSC_SME_ID;
FORM_METHOD = "POST";
TITLE = "SMSC Rescheduling";
MENU0_SELECTION = "MENU0_SMSC_Reshedule";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		STATUS.append("Ok");
		break;
  case Index.RESULT_ADD:
    response.sendRedirect("add.jsp");
    return;
  case Index.RESULT_EDIT:
    response.sendRedirect("edit.jsp?editKey=" + URLEncoder.encode(bean.getEditKey(), "UTF-8"));
    return;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%@
include file="/WEB-INF/inc/html_3_header.jsp"%><%@
include file="/WEB-INF/inc/collapsing_tree.jsp"%><%
page_menu_begin(out);
page_menu_button(out, "mbAdd",  "Add",  "Add new reshedule policy", !bean.isAllErrorsAssigned());
page_menu_button(out, "mbDelete",  "Delete",  "Delete all checked reshedules", "return confirm('Are you sure to delete all checked reshedules?')");
page_menu_space(out);
page_menu_button(out, "mbSave",  "Save",  "Save current shedules to config");
page_menu_button(out, "mbReset", "Reset", "Read reshedules from config");
page_menu_end(out);
%><%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<input type=hidden name=editKey>
<script>
function clickEdit(value)
{
  opForm.all.jbutton.value = "mbEdit";
  opForm.all.jbutton.name = "mbEdit";
  opForm.all.editKey.value = value;
  opForm.submit();
  return false;
}
</script>
<%final int WIDTH = 3;%>
<table class=list>
<col width=1px>
<col width=33%>
<%
  for (int i=0; i < WIDTH; i++)
  {
    %><col align=right><col align=left><%
  }
%>
<tr>
  <th>&nbsp;</th>
  <th>reshedule policy</th>
  <th colspan=<%=WIDTH*2%>>errors</th>
</tr>
<%
  int rowN = 1;
  for (Iterator i = bean.getReshedules().iterator(); i.hasNext();) {
    String reshedule = (String) i.next();
    Collection resheduleErrors = bean.getResheduleErrors(reshedule);
    int rows = resheduleErrors.size() / WIDTH + (resheduleErrors.size() % WIDTH > 0 ? 1 : 0);
    if (rows == 0)
      rows = 1;
    %><tr class=row<%=(++rowN)&1%>><td rowspan=<%=rows%>><input class=check type=checkbox name=checkedShedules value="<%=reshedule%>" <%=bean.isSheduleChecked(reshedule) ? "checked" : ""%>></td><td rowspan=<%=rows%>><a href="#" onClick="return clickEdit('<%=reshedule%>');"><%=reshedule%></a></td><%
    int count = 0;
    for (Iterator j = resheduleErrors.iterator(); j.hasNext();) {
      String errorCode = (String) j.next();
      String errorCodeString = bean.getErrorString(request.getLocale(), errorCode);
      %><td>(<%=errorCode%>)</td><td><%=errorCodeString != null ? errorCodeString : ""%></td><%
      if (++count % WIDTH == 0)
      {
        %></tr><tr class=row<%=rowN&1%>><%
      }
    }
    final int lastColSpan = rows*WIDTH - resheduleErrors.size();
    if (lastColSpan > 0)
    {
      %><td colspan=<%=lastColSpan*2%>>&nbsp;</td><%
    }
    %></tr><%
  }
%>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbAdd",  "Add",  "Add new reshedule policy", !bean.isAllErrorsAssigned());
page_menu_button(out, "mbDelete",  "Delete",  "Delete all checked reshedules", "return confirm('Are you sure to delete all checked reshedules?')");
page_menu_space(out);
page_menu_button(out, "mbSave",  "Save",  "Save current shedules to config");
page_menu_button(out, "mbReset", "Reset", "Read reshedules from config");
page_menu_end(out);
%><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>