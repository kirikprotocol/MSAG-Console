<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ page import="ru.novosoft.smsc.wsme.*,
                 ru.novosoft.smsc.wsme.beans.*,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.util.List,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.smsc.profiles.ProfilesFilter"%>
<jsp:useBean id="wsme_vf_bean" scope="session" class="ru.novosoft.smsc.wsme.beans.WSmeVisitorsFilterFormBean"/>
<%
  WSmeVisitorsFilterFormBean bean = wsme_vf_bean;
%>
<jsp:setProperty name="wsme_vf_bean" property="*"/>
<%
  TITLE="Welcome SME Visitors Filter";
  MENU0_SELECTION = "MENU0_SERVICES";
  //MENU1_SELECTION = "WSME_INDEX";

  int beanResult = bean.RESULT_OK;
  switch(beanResult = bean.process(errorMessages, loginedUserPrincipal))
  {
    case WSmeFormBean.RESULT_DONE:
      response.sendRedirect("visitors.jsp");
      return;
    case WSmeFormBean.RESULT_OK:
      STATUS.append("Ok");
      break;
    case WSmeFormBean.RESULT_ERROR:
      STATUS.append("<span class=CF00>Error</span>");
      break;
    default:
      STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<div class=page_subtitle>Masks</div>
<table class=list cellspacing=0>
<col width="15%" align=right>
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Masks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
  int row = 0;
  for (int i=0; i<bean.getMasks().length; i++)
  {
%>
<tr class=row<%=(row++)&1%>>
	<td>&nbsp;</td>
	<td><input class=txt name=masks value="<%=bean.getMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(row++)&1%>>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to filter"></td>
	<td><input class=txt name=masks validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "return noValidationSubmit(this);");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "return noValidationSubmit(this);");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>


