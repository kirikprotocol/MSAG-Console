<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.infosme.beans.Options,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.infosme.beans.Index"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = Constants.INFO_SME_ID;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.RESULT_OK;
	switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
	{
		case Options.RESULT_APPLY:
			response.sendRedirect("index.jsp");
			return;
		case Options.RESULT_OPTIONS:
			response.sendRedirect("options.jsp");
			return;
		case Options.RESULT_DRIVERS:
			response.sendRedirect("drivers.jsp");
			return;
		case Options.RESULT_PROVIDERS:
			response.sendRedirect("providers.jsp");
			return;
		case Options.RESULT_TASKS:
			response.sendRedirect("tasks.jsp");
			return;
		case Options.RESULT_SHEDULES:
			response.sendRedirect("shedules.jsp");
			return;
		case Options.RESULT_DONE:
			response.sendRedirect("index.jsp");
			return;
		case Options.RESULT_OK:
			STATUS.append("Ok");
			break;
		case Options.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<col width="1%">
<col width="99%">
<% int rowN = 0;%>
<tr class=row<%=rowN++&1%>>
  <th>Service type</th>
  <td><input class=txt name=svcType value="<%=bean.getSvcType()%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Protocol ID</th>
  <td><input class=txt name=protocolId value="<%=bean.getProtocolId()%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Thread pool max</th>
  <td><input class=txt name=threadPoolMax value="<%=bean.getThreadPoolMax()%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Thread pool init</th>
  <td><input class=txt name=threadPoolInit value="<%=bean.getThreadPoolInit()%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC host</th>
  <td><input class=txt name=smscHost value="<%=bean.getSmscHost()%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC port</th>
  <td><input class=txt name=smscPort value="<%=bean.getSmscPort()%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC SID</th>
  <td><input class=txt name=smscSid value="<%=bean.getSmscSid()%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection timeout</th>
  <td><input class=txt name=smscTimeout value="<%=bean.getSmscTimeout()%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection password</th>
  <td><input class=txt name=smscPassword value="<%=bean.getSmscPassword()%>"></td>
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