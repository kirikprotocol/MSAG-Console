<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.ProviderEdit,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.ProviderEdit" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = Constants.INFO_SME_ID;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
	switch(beanResult)
	{
		case ProviderEdit.RESULT_DONE:
			response.sendRedirect("providers.jsp");
			return;
		default:
      {
        %><%@ include file="inc/menu_switch.jsp"%><%
      }
	}
  int rowN = 0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=create value=<%=bean.isCreate()%>>
<input type=hidden name=oldProvider value="<%=bean.getOldProvider()%>">
<table class=properties_list>
<col width="10%">
<tr class=row<%=rowN++&1%>>
  <th>Provider name</th>
  <td><input class=txt name=provider value="<%=StringEncoderDecoder.encode(bean.getProvider())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Data source type</th>
  <td><input class=txt name=type value="<%=StringEncoderDecoder.encode(bean.getType())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Connections</th>
  <td><input class=txt name=connections value="<%=StringEncoderDecoder.encode(bean.getConnections())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB instance</th>
  <td><input class=txt name=dbInstance value="<%=StringEncoderDecoder.encode(bean.getDbInstance())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB user name</th>
  <td><input class=txt name=dbUserName value="<%=StringEncoderDecoder.encode(bean.getDbUserName())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB user password</th>
  <td><input class=txt name=dbUserPassword value="<%=StringEncoderDecoder.encode(bean.getDbUserPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=watchdog>Watchdog</label></th>
  <td><input class=check type=checkbox name=watchdog id=watchdog value=true <%=bean.isWatchdog() ? "checked" : ""%>></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbDone",  "Done",  "Done editing");
page_menu_button(out, "mbCancel", "Cancel", "Cancel changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>