<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.infosme.beans.Options,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.infosme.beans.Index,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*"%>
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
  <td><input class=txt name=svcType value="<%=StringEncoderDecoder.encode(bean.getSvcType())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Protocol ID</th>
  <td><input class=txt name=protocolId value="<%=StringEncoderDecoder.encode(bean.getProtocolId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Internal data provider</th>
  <td><select name=dataProviderDsInternal><%
    for (Iterator i = bean.getAllDataProviders().iterator(); i.hasNext();) {
      String providerName = (String) i.next();
      String providerNameEnc = StringEncoderDecoder.encode(providerName);
      %><option value="<%=providerNameEnc%>" <%=providerName.equals(bean.getDataProviderDsInternal()) ? "selected" : ""%>><%=providerNameEnc%></option><%
    }
  %></select></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Thread pool max</th>
  <td><input class=txt name=threadPoolMax value="<%=StringEncoderDecoder.encode(bean.getThreadPoolMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Thread pool init</th>
  <td><input class=txt name=threadPoolInit value="<%=StringEncoderDecoder.encode(bean.getThreadPoolInit())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC host</th>
  <td><input class=txt name=smscHost value="<%=StringEncoderDecoder.encode(bean.getSmscHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC port</th>
  <td><input class=txt name=smscPort value="<%=StringEncoderDecoder.encode(bean.getSmscPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC SID</th>
  <td><input class=txt name=smscSid value="<%=StringEncoderDecoder.encode(bean.getSmscSid())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection timeout</th>
  <td><input class=txt name=smscTimeout value="<%=StringEncoderDecoder.encode(bean.getSmscTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection password</th>
  <td><input class=txt name=smscPassword value="<%=StringEncoderDecoder.encode(bean.getSmscPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks switch timeout</th>
  <td><input class=txt name=tasksSwitchTimeout value="<%=StringEncoderDecoder.encode(bean.getTasksSwitchTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks task tables prefix</th>
  <td><input class=txt name=tasksTaskTablesPrefix value="<%=StringEncoderDecoder.encode(bean.getTasksTaskTablesPrefix())%>"></td>
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