<%@
 include file="/WEB-INF/inc/code_header.jsp"%><%@
 page import="ru.novosoft.smsc.jsp.smsc.smsc_service.Index,
				  ru.novosoft.smsc.admin.service.ServiceInfo,
				  ru.novosoft.smsc.admin.Constants,
				  java.io.IOException,
				  ru.novosoft.smsc.util.StringEncoderDecoder,
				  java.util.Iterator,
				  ru.novosoft.smsc.jsp.SMSCErrors,
				  ru.novosoft.smsc.jsp.SMSCJspException"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smsc_service.Logging"
/><jsp:setProperty name="bean" property="*"/><%
	FORM_METHOD = "POST";
	TITLE = "SMSC";
	MENU0_SELECTION = "MENU0_SMSC_LOGGING";
	switch (bean.process(appContext, errorMessages, loginedUserPrincipal, request.getParameterMap()))
	{
		case Index.RESULT_DONE:
			response.sendRedirect(CPATH+"/index.jsp");
			return;
		case Index.RESULT_OK:
			STATUS.append("Ok");
			break;
		case Index.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
	}
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<table class=secRep cellspacing=1 width="100%">
<col width="80%" align=left>
<col width="20%" align=left>
<thead>
<tr>
</tr>
</thead>
<tbody>
<%
	int rowN = 0;
	for (Iterator i = bean.getCategoryNames().iterator(); i.hasNext();)
	{
		String catName = (String) i.next();
		final String categoryPriority = bean.getCategoryPriority(catName);
		final String catNameEnc = StringEncoderDecoder.encode(catName);
		%><tr class=row<%=(rowN++)&1%>>
			<th class=label style="text-align:left;"><%=catNameEnc%>&nbsp;</th>
			<td><select name="<%=bean.catParamNamePrefix + catNameEnc%>">
 				<option value="FATAL"  <%="FATAL".equalsIgnoreCase(categoryPriority)  ? " selected" : ""%>>FATAL</option>
 				<option value="ALERT"  <%="ALERT".equalsIgnoreCase(categoryPriority)  ? " selected" : ""%>>ALERT</option>
 				<option value="CRIT"   <%="CRIT".equalsIgnoreCase(categoryPriority)   ? " selected" : ""%>>CRIT</option>
 				<option value="ERROR"  <%="ERROR".equalsIgnoreCase(categoryPriority)  ? " selected" : ""%>>ERROR</option>
 				<option value="WARN"   <%="WARN".equalsIgnoreCase(categoryPriority)   ? " selected" : ""%>>WARN</option>
 				<option value="NOTICE" <%="NOTICE".equalsIgnoreCase(categoryPriority) ? " selected" : ""%>>NOTICE</option>
 				<option value="INFO"   <%="INFO".equalsIgnoreCase(categoryPriority)   ? " selected" : ""%>>INFO</option>
 				<option value="DEBUG"  <%="DEBUG".equalsIgnoreCase(categoryPriority)  ? " selected" : ""%>>DEBUG</option>
 				<option value="NOTSET" <%="NOTSET".equalsIgnoreCase(categoryPriority) ? " selected" : ""%>>not set</option>
			</select></td>
		</tr><%
	}
%>
</tbody>
</table>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save">
<input class=btn type=submit name=mbCancel value="Cancel" title="Cancel">
</div><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>