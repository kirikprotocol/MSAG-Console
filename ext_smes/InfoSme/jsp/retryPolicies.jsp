<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.net.URLEncoder, java.util.Iterator,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem"%>
<%@ page import="ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataItem" %>
<%@ page import="ru.novosoft.smsc.infosme.beans.*" %>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.RetryPolicyListBean" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";

	int beanResult = bean.process(request);
	switch(beanResult)
	{
    case RetryPolicyListBean.RESULT_EDIT:
      response.sendRedirect("retryPolicyEdit.jsp?name=" + URLEncoder.encode(bean.getEdit(), "UTF-8"));
      return;
    case RetryPolicyListBean.RESULT_ADD:
      response.sendRedirect("retryPolicyEdit.jsp?create=true");
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
<input type=hidden name=edit id=edit>
<input type=hidden name=sort id=sort>
<script type="text/javascript">
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;
	opForm.submit();
	return false;
}
</script>
<%
  QueryResultSet policies = bean.getPolicies();
  if (policies.size() == 0)
  {%>No policies found<%}
  else
  {%>
    <table class=list cellspacing=0>
    <col width="1%">
    <col width="99%"            > <!--name-->
    <tr>
      <th>&nbsp;</th>
      <th><a href="#" <%=bean.getSort().endsWith("name")           ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Policy name"            onclick='return setSort("name")'           >Policy</a></th>
    </tr>
    <%
    int rowN = 0;
    for (Iterator i = policies.iterator(); i.hasNext();) {
      RetryPolicyDataItem policyDataItem = (RetryPolicyDataItem) i.next();

      String name = policyDataItem.getName();

      String nameEnc = StringEncoderDecoder.encode(name);

      %>
      <tr class=row<%=rowN++&1%>>
        <td><input class=check type=checkbox name=checked value="<%=name%>" <%=bean.isPolicyChecked(name) ? "checked" : ""%> onclick="checkCheckboxesForMbDeleteButton();"></td>
        <td><a href="javascript:editSomething('<%=name%>')" title="Edit policy"><%=nameEnc%></a></td>
      </tr>
    <%
    }
    %></table><%
  }%>
</div><%
page_menu_begin(out);
if (bean.isSmeRunning()) {
  page_menu_button(session, out, "mbAdd",    "common.buttons.add",    "infosme.hint.add_policy");
  page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "infosme.hint.del_policies", getLocString("infosme.confirm.del_policies"));
}
page_menu_space(out);
page_menu_end(out);
%>
<script type="text/javascript">
<%if (bean.isSmeRunning()) {%>checkCheckboxesForMbDeleteButton();<%}%>
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>