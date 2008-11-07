<%@ page import="ru.novosoft.smsc.infosme.beans.RetryPolicyEdit" %>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper" %>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.RetryPolicyEdit" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";

	int beanResult = bean.process(request);
	switch(beanResult)
	{
    case RetryPolicyEdit.RESULT_SAVE:
    case RetryPolicyEdit.RESULT_CANCEL:
      response.sendRedirect("retryPolicies.jsp");
      return;
		default:
      {
        %><%@ include file="inc/menu_switch.jsp"%><%
      }
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
  <input type=hidden name=create id=create value="<%=bean.isCreate()%>">

  <table class=properties_list cellspacing=0>
    <col width="1%">
    <col width="99%">
    <% int rowN = 0;%>
    <tr class=row<%=rowN++&1%>>
      <th>Name</th>
      <td><input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>" <%=bean.isCreate()?"" : "READONLY"%> validation="nonEmpty"></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th>Defalt Policy:</th>
      <td><input class=txt name=defaultPolicy value="<%=bean.getDefaultPolicy()%>" validation="positive"></td>
    </tr>
 </table>

  <%{ final DynamicTableHelper tableHelper = bean.getHelper();%>
  <%@ include file="/WEB-INF/inc/dynamic_table.jsp"%>
  <%}%>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",   "common.buttons.done",  "infosme.hint.done_editing");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "infosme.hint.cancel_changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

