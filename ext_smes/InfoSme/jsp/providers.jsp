<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.infosme.beans.Providers,
					  ru.novosoft.smsc.infosme.beans.Index,
                 java.util.Collection,
                 java.util.Iterator,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Providers" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = Constants.INFO_SME_ID;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
	switch(beanResult)
	{
    case Providers.RESULT_REFRESH:
      response.sendRedirect("providers.jsp?refreshed=true&sort=" + URLEncoder.encode(bean.getSort(), "UTF-8"));
      return;
    case Providers.RESULT_EDIT:
      response.sendRedirect("providerEdit.jsp?provider=" + URLEncoder.encode(bean.getEdit(), "UTF-8"));
      return;
    case Providers.RESULT_ADD:
      response.sendRedirect("providerEdit.jsp?create=true");
      return;
		default:
      {
        %><%@ include file="inc/menu_switch.jsp"%><%
      }
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=edit>
<input type=hidden name=sort>
<script>
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;
	opForm.submit();
	return false;
}
</script><%
  Collection providerNames = bean.getProviderNames();
  if (providerNames.size() > 0)
  {
%>
<table class=list cellspacing=0>
<col width="1%">
<tr>
  <th>&nbsp;</th>
  <th><a href="#" <%=bean.getSort().endsWith("provider") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by data provider name" onclick='return setSort("provider")'>Data provider name</a></th>
</tr>
<%
    int rowN = 0;
    for (Iterator i = providerNames.iterator(); i.hasNext();) {
      String providerName = (String) i.next();
      String providerNameEnc = StringEncoderDecoder.encode(providerName);
      %><tr class=row<%=rowN++&1%>>
        <td><input class=check type=checkbox name=checkedProviders value="<%=providerNameEnc%>" <%=bean.isProviderChecked(providerName) ? "checked" : ""%> onclick="checkCheckboxesForMbDeleteButton();"></td>
        <td><a href="#" title="Edit provider" onClick='return editSomething("<%=providerNameEnc%>");'><%=providerNameEnc%></a></td>
      </tr>
<%
    }
%>
</table>
<%} else {%>
No providers found.
<%}%>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbAdd",    "Add",    "Add new provider");
page_menu_button(out, "mbDelete", "Delete", "Delete checked provider(s)", "return confirm('Are you sure to delete all checked providers?');");
page_menu_space(out);
page_menu_end(out);
%>
<script>
checkCheckboxesForMbDeleteButton();
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>