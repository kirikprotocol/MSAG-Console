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
              java.util.Locale,
              java.net.URLEncoder"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.reshedule.Index"
/><jsp:setProperty name="bean" property="*"/><%
//ServiceIDForShowStatus = Constants.SMSC_SME_ID;
FORM_METHOD = "POST";
TITLE = getLocString("reschedule.title");
MENU0_SELECTION = "MENU0_SMSC_Reshedule";
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		break;
  case Index.RESULT_ADD:
    response.sendRedirect("add.jsp");
    return;
  case Index.RESULT_EDIT:
    response.sendRedirect("edit.jsp?editKey=" + URLEncoder.encode(bean.getEditKey()));
    return;
	case Index.RESULT_ERROR:
		break;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%@
include file="/WEB-INF/inc/html_3_header.jsp"%><%@
include file="/WEB-INF/inc/collapsing_tree.jsp"%><%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",    "common.buttons.add",  "reschedule.addHint", !bean.isAllErrorsAssigned());
page_menu_button(session, out, "mbDelete", "common.buttons.delete",  "reschedule.deleteHint", "return confirm('"+getLocString("reschedule.deleteConfirm")+"')");
page_menu_space(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "reschedule.saveHint");
page_menu_button(session, out, "mbReset", "common.buttons.reset", "reschedule.resetHint");
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
<%final int WIDTH = 1;
  String defaultReshedule = bean.getDefaultReshedule();
  if (defaultReshedule == null || defaultReshedule.trim().length() == 0)
    defaultReshedule = "&lt;not specified&gt;";
%>
<div class=page_subtitle><%=getLocString("reschedule.subTitle")%></div><br>
<table class=list>
<col width=1px>
<tr class=row0>
  <td>&nbsp;</td>
  <td><a href="#" onClick="return clickEdit('<%=bean.getDefaultResheduleName()%>');"><%=defaultReshedule%></a></td>
</tr>
<tr  class=row1>
<td>&nbsp;</td>
<td><%=getLocString("reschedule.defaultPolicy")%></td></tr>
<%
 if (bean.getReshedules() != null)
 {
  for (Iterator i = bean.getReshedules().iterator(); i.hasNext();) {
    String reshedule = (String) i.next();
    Collection resheduleErrors = bean.getResheduleErrors(reshedule);
    int rows = resheduleErrors.size() / WIDTH + (resheduleErrors.size() % WIDTH > 0 ? 1 : 0);
    if (rows == 0)
      rows = 1;
    %><tr class=row0>
    <td><input class=check type=checkbox name=checkedShedules value="<%=reshedule%>" <%=bean.isSheduleChecked(reshedule) ? "checked" : ""%>></td>
    <td><a href="#" onClick="return clickEdit('<%=reshedule%>');"><%=reshedule%></a></td></tr>

 <%
    //int count = 0;
    if (resheduleErrors.size() <= 0 )
    {
 %>
     <tr class=row1><td>&nbsp;</td><td><i><%=getLocString("common.util.noSelectedErrors")%></i></td></tr>
 <%
    }
    else {
      %><tr  class=row1>
      <td colspan=2><table style="border:none"><col width=80px><%
      for (Iterator j = resheduleErrors.iterator(); j.hasNext();) {
        String errorCode = (String) j.next();
        String errorCodeString = bean.getErrorString(errorCode);
        %><tr style="border:none">
      <td style="border:none" align=right>(<%=errorCode%>)</td><td style="border:none"><%=errorCodeString != null ? errorCodeString : ""%></td></tr><%
      }
      %></table></td></tr><%
    }
  }
 }    
%>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",    "common.buttons.add",  "reschedule.addHint", !bean.isAllErrorsAssigned());
page_menu_button(session, out, "mbDelete", "common.buttons.delete",  "reschedule.deleteHint", "return confirm('"+getLocString("reschedule.deleteConfirm")+"')");
page_menu_space(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "reschedule.saveHint");
page_menu_button(session, out, "mbReset", "common.buttons.reset", "reschedule.resetHint");
page_menu_end(out);
%><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>