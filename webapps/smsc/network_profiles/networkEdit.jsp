<%@ page import="ru.novosoft.smsc.jsp.smsc.region.RegionEditBean"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.network_profiles.NetworkProfileEditBean" %>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper" %>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.smsc.network_profiles.NetworkProfileEditBean" />
<jsp:setProperty name="bean" property="*"/>

<%
  TITLE=getLocString("infosme.title");
  MENU0_SELECTION = "MENU0_SERVICES";

  bean.getMasksHelper().processRequest(request);

  int beanResult = bean.process(request);
	switch(beanResult)
	{
		case NetworkProfileEditBean.RESULT_SAVE:
		case NetworkProfileEditBean.RESULT_CANCEL:
			response.sendRedirect("networks.jsp");
			return;
		default:
	}
  int rowN = 0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<div class=content>
  <input type=hidden name=oldName id=oldName value="<%=bean.getOldName() == null ? "" : bean.getOldName()%>">
  <table class=properties_list>
    <col width="1%">
    <col width="49%">
    <col width="50%">
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("network.name")%></th>
      <td>
        <input type=txt name=name id=name value="<%=bean.getName() == null ? "" : bean.getName()%>" validation="nonEmpty" onkeyup="resetValidation(this)">

        </td>
      <td>&nbsp;</td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("network.abonentStatusMethod")%></th>
      <td>
        <div class="select">
          <select name="abonentStatusMethod" id="abonentStatusMethod">
            <%
              {
                Iterator i = bean.getAbonentStateMethods().iterator();
                while(i.hasNext()) {
                  String o = (String)i.next();
            %>
            <option value="<%=o%>" <%=o.equals(bean.getAbonentStatusMethod()) ? "selected" : ""%>><%=o%></option>
            <%
                }
              }
            %>
          </select>
        </div>
      </td>
      <td>&nbsp;</td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("network.ussdOpenDestRef")%></th>
      <td>
        <div class="select">
          <select name="ussdOpenDestRef" id="ussdOpenDestRef">
            <%
              {
                Iterator i = bean.getUssdOpenDestRefs().iterator();
                while(i.hasNext()) {
                  String o = (String)i.next();
            %>
            <option value="<%=o%>" <%=o.equals(bean.getUssdOpenDestRef()) ? "selected" : ""%>><%=o%></option>
            <%
                }
              }
            %>
          </select>
        </div>
        </td>
      <td>&nbsp;</td>
    </tr>
    <tr class=row<%=rowN++&1%>>
    <td colspan="2">
        <%  final DynamicTableHelper tableHelper = bean.getMasksHelper();%>
          <%@ include file="/WEB-INF/inc/dynamic_table.jsp"%>
    </td>
</tr>
  </table>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbApply", "common.buttons.apply", "common.buttons.apply");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>