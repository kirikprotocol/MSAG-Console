<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.mcisme.beans.Template,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  java.util.Set, java.util.Iterator,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  java.util.Collection, ru.novosoft.smsc.mcisme.beans.MCISmeBean,
                 ru.novosoft.smsc.infosme.beans.TaskEdit"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Template" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE="Missed Calls Info SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
	switch(beanResult)
	{
		case MCISmeBean.RESULT_DONE:
			response.sendRedirect("templates.jsp");
			return;
		default: {
      %><%@ include file="inc/menu_switch.jsp"%><%
    }
	}
  int rowN=1;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%><%
  boolean informTemplate = bean.isInformTemplate();
  boolean createTemplate = bean.isCreateTemplate();
  String templatePageSubtitle = ((informTemplate) ? "Inform":"Notify")+
       " template "+((createTemplate) ? "create":"modify");
%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=informTemplate  value=<%=bean.isInformTemplate()%>>
<input type=hidden name=createTemplate  value=<%=bean.isCreateTemplate()%>>
<input type=hidden name=oldTemplateName value="<%=bean.getOldTemplateName()%>">
<div class=page_subtitle><%= StringEncoderDecoder.encode(templatePageSubtitle)%></div>
<table class=properties_list cellspacing=0 cellpadding=4 width="100%">
<col width="1%" align=left>
<col width="99%" align=left>
<tr class=row<%=(rowN++)&1%>>
  <th>Template id</th>
  <td><input class=txt name=templateId value="<%=bean.getTemplateId()%>" maxlength="10" validation="id" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th>Template name</th>
  <td><input class=txt name=templateName value="<%=StringEncoderDecoder.encode(bean.getTemplateName())%>"></td>
</tr>
<%if (informTemplate) { %>
<tr class=row<%=(rowN++)&1%>>
  <th>Group callers</th>
  <td><input class=check type=checkbox name=group value=true <%=bean.isGroup() ? "checked" : ""%>></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
  <th valign=top>Message</th>
  <td valign=top><textarea name=message><%=StringEncoderDecoder.encode(bean.getMessage())%></textarea></td>
</tr>
<%if (informTemplate) { %>
<tr class=row<%=(rowN++)&1%>>
  <th valign=top>Single row</th>
  <td valign=top><textarea name=singleRow><%=StringEncoderDecoder.encode(bean.getSingleRow())%></textarea></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th valign=top>Multi row</th>
  <td valign=top><textarea name=multiRow><%=StringEncoderDecoder.encode(bean.getMultiRow())%></textarea></td>
</tr>
<%}%>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbDone",   "Done",   "Submit changes");
page_menu_button(out, "mbCancel", "Cancel", "Discard changes");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>