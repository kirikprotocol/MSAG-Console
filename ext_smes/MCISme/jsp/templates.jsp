<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants, ru.novosoft.smsc.mcisme.beans.Templates,
					       ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder, ru.novosoft.smsc.mcisme.beans.MCISmeBean,
                 java.net.URLEncoder, java.util.*, ru.novosoft.smsc.util.Functions"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Templates" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE=getLocString("mcisme.title");
  MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
  switch (beanResult)
  {
    case MCISmeBean.RESULT_INFORM_ADD:
      response.sendRedirect("template.jsp?informTemplate=true&createTemplate=true&templateName=");
      return;
    case MCISmeBean.RESULT_INFORM_EDIT:
      response.sendRedirect("template.jsp?informTemplate=true&createTemplate=false&templateName="+
                            URLEncoder.encode(bean.getEditTemplate(), Functions.getLocaleEncoding()));
      return;
    case MCISmeBean.RESULT_NOTIFY_ADD:
      response.sendRedirect("template.jsp?informTemplate=false&createTemplate=true&templateName=");
      return;
    case MCISmeBean.RESULT_NOTIFY_EDIT:
      response.sendRedirect("template.jsp?informTemplate=false&createTemplate=false&templateName="+
                            URLEncoder.encode(bean.getEditTemplate(), Functions.getLocaleEncoding()));
      return;
		default: {
      %><%@ include file="inc/menu_switch.jsp"%><%
    }
  }
int rowN=1;
List informList = bean.getInformTemplates();
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<script>
function editMCISmeTemplate(editObjectName, editButtonName)
{
  opForm.all.jbutton.name       = editButtonName;
  opForm.all.jbutton.value      = editButtonName;
  opForm.all.editTemplate.value = editObjectName;
  opForm.submit();
  return false;
}
function checkCheckboxesForInformTemplate() {
  return checkCheckboxes(opForm.all.mbInformDelete);
}
function checkCheckboxesForNotifyTemplate() {
  return checkCheckboxes(opForm.all.mbNotifyDelete);
}
</script>
<div class=content>
<input type=hidden name=editTemplate value="<%=bean.getEditTemplate()%>">
<div class=page_subtitle><%= getLocString("mcisme.subtitle.inform_template")%></div>
<table id="inform_table" cellspacing=5>
<tr><td>
  <label><%= getLocString("mcisme.label_default_template")%></label>
  <select name="defaultInformId" id="defaultInformId">
  <%for (Iterator i=informList.iterator(); i.hasNext();) {
    Templates.Identity id = (Templates.Identity)i.next();%>
    <option value="<%= id.id%>" <%= (id.id == bean.getDefaultInformId()) ? "selected":""%>><%=
      StringEncoderDecoder.encode(id.name)%></option><%
  }%>
  </select>
</td></tr>
<tr><td>
  <table class=list cellspacing=1 width="100%">
  <col width="1%"  align=left><col width="5%"  align=left><col width="94%" align=left>
  <tr class=row<%=(rowN++)&1%>>
    <th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt="&nbsp;"></th>
    <th nowrap valign=top style="text-align: left">ID</th>
    <th nowrap valign=top style="text-align: left">Template name</th>
  </tr>
  <%for (Iterator i=informList.iterator(); i.hasNext();) {
    Templates.Identity id = (Templates.Identity)i.next();
    String encName = StringEncoderDecoder.encode(id.name);%>
    <tr class=row<%=(rowN++)&1%>>
      <td><input class=check type=checkbox name=informChecked
                 value="<%=encName%>" <%=bean.isInformTemplateChecked(id.name) ? "checked":""%>
                 onClick="checkCheckboxesForInformTemplate();"></td>
      <td><%= id.id%></td>
      <td><a href="#" title="<%= getLocString("mcisme.hint.inform_template_edit")%>"
             onClick='return editMCISmeTemplate("<%=encName%>", "mbInformEdit");'><%=encName%></a></td>
    </tr><%
  }%>
  </table>
</td></tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbInformAdd",    "mcisme.button.add_template", "mcisme.hint.add_inform_template");
page_menu_button(session, out, "mbInformDelete", "mcisme.button.del_template", "mcisme.button.del_inform_template",
                               "return confirm('"+getLocString("mcisme.confirm.del_inform_template")+"');");
page_menu_space(out);
page_menu_end(out);
rowN = 1;
List notifyList = bean.getNotifyTemplates();
%>
<script>checkCheckboxesForInformTemplate();</script>
<div class=content>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.notify_template")%></div>
<table id="notify_table" cellspacing=5>
<tr><td>
  <label><%= getLocString("mcisme.label_default_template")%></label>
  <select name="defaultNotifyId" id="defaultNotifyId">
  <%for (Iterator i=notifyList.iterator(); i.hasNext();) {
      Templates.Identity id = (Templates.Identity)i.next();%>
      <option value="<%= id.id%>" <%= (id.id == bean.getDefaultNotifyId()) ? "selected":""%>><%=
        StringEncoderDecoder.encode(id.name)%></option><%
  }%>
  </select>
</td></tr>
<tr><td>
  <table class=list cellspacing=1 width="100%">
  <col width="1%"  align=left><col width="5%"  align=left><col width="94%" align=left>
  <tr class=row<%=(rowN++)&1%>>
    <th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt="&nbsp;"></th>
    <th nowrap valign=top style="text-align: left">ID</th>
    <th nowrap valign=top style="text-align: left">Template name</th>
  </tr>
  <%for (Iterator i=notifyList.iterator(); i.hasNext();) {
    Templates.Identity id = (Templates.Identity)i.next();
    String encName = StringEncoderDecoder.encode(id.name);%>
    <tr class=row<%=(rowN++)&1%>>
      <td><input class=check type=checkbox name=notifyChecked
                 value="<%=encName%>" <%=bean.isNotifyTemplateChecked(id.name) ? "checked":""%>
                 onclick="checkCheckboxesForNotifyTemplate();"></td>
      <td><%= id.id%></td>
      <td><a href="#" title="<%= getLocString("mcisme.hint.notify_template_edit")%>"
             onClick='return editMCISmeTemplate("<%=encName%>", "mbNotifyEdit");'><%=encName%></a></td>
    </tr><%
  }%>
  </table>
</td></tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbNotifyAdd",    "mcisme.button.add_template", "mcisme.button.add_notify_template");
page_menu_button(session, out, "mbNotifyDelete", "mcisme.button.del_template", "mcisme.button.del_notify_template",
                               "return confirm('"+getLocString("mcisme.confirm.del_notify_template")+"');");
page_menu_space(out);
page_menu_end(out);%>
<script>checkCheckboxesForNotifyTemplate();</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>