<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.mcisme.beans.Templates,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.mcisme.beans.MCISmeBean,
                 java.net.URLEncoder,
                 java.util.*"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Templates" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE="Missed Calls Info SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
  switch (beanResult)
  {
    case MCISmeBean.RESULT_INFORM_ADD:
      response.sendRedirect("template.jsp?informTemplate=true&createTemplate=true&templateName=");
      return;
    case MCISmeBean.RESULT_INFORM_EDIT:
      response.sendRedirect("template.jsp?informTemplate=true&createTemplate=false&templateName="+
                            URLEncoder.encode(bean.getEditTemplate(), "UTF-8"));
      return;
    case MCISmeBean.RESULT_NOTIFY_ADD:
      response.sendRedirect("template.jsp?informTemplate=false&createTemplate=true&templateName=");
      return;
    case MCISmeBean.RESULT_NOTIFY_EDIT:
      response.sendRedirect("template.jsp?informTemplate=false&createTemplate=false&templateName="+
                            URLEncoder.encode(bean.getEditTemplate(), "UTF-8"));
      return;
		default: {
      %><%@ include file="inc/menu_switch.jsp"%><%
    }
  }
int rowN=1;
List informList = bean.getInformTemplates();
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
<div class=page_subtitle>Inform abonents templates</div>
<table id="inform_table" cellspacing=5>
<tr><td>
  <label>Default template:</label>
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
    Templates.Identity id = (Templates.Identity)i.next();%>
    <tr class=row<%=(rowN++)&1%>>
      <td>&nbsp;</td> <!-- TODO: checkboxes -->
      <td><%= id.id%></td>
      <td><%= StringEncoderDecoder.encode(id.name)%></td>
    </tr><%
  }%>
  </table>
</td></tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbInformAdd",    "Add template",  "Add new inform template");
page_menu_button(out, "mbInformDelete", "Delete template(s)", "Delete selected inform template(s)");
page_menu_space(out);
page_menu_end(out);
rowN = 1;
List notifyList = bean.getNotifyTemplates();
%><div class=content>
<div class=page_subtitle>Notify callers templates</div>
<table id="notify_table" cellspacing=5>
<tr><td>
  <label>Default template:</label>
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
    Templates.Identity id = (Templates.Identity)i.next();%>
    <tr class=row<%=(rowN++)&1%>>
      <td>&nbsp;</td> <!-- TODO: checkboxes -->
      <td><%= id.id%></td>
      <td><%= StringEncoderDecoder.encode(id.name)%></td>
    </tr><%
  }%>
  </table>
</td></tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbNotifyAdd",    "Add template",  "Add new notify template");
page_menu_button(out, "mbNotifyDelete", "Delete template(s)", "Delete selected notify template(s)");
page_menu_space(out);
page_menu_end(out);%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>