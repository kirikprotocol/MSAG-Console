<%@ page import="ru.novosoft.smsc.jsp.smsc.network_profiles.NetworkProfilesBean" %>
<%@ page import="ru.novosoft.smsc.jsp.util.tables.impl.network_profiles.NetworkProfileDataItem" %>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.smsc.network_profiles.NetworkProfilesBean" />
<jsp:setProperty name="bean" property="*"/>

<%
  TITLE=getLocString("network_profiles.title");
	MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(request);
  switch(beanResult)
	{
		case NetworkProfilesBean.RESULT_EDIT:
      response.sendRedirect("networkEdit.jsp?network=" + bean.getEdit());
      return;
    case NetworkProfilesBean.RESULT_ADD:
      response.sendRedirect("networkEdit.jsp");
      return;
		default:
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<div class=content>
  <div class=page_subtitle><%=getLocString("network.default")%></div>
  <input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
  <input type=hidden name=sort id=sort value="<%=bean.getSort()%>">
  <input type=hidden name=edit id=edit>
  <table>
    <col width="1%">
    <col width="49%">
    <tr>
      <td nowrap="true"><%=getLocString("network.abonentStatusMethod")%>:</td>
      <td><input class=txt name=defaultAbonentStatusMethod id=defaultAbonentStatusMethod value="<%=bean.getDefaultAbonentStatusMethod() == null ? "" : bean.getDefaultAbonentStatusMethod()%>"></td>
    </tr>
    <tr>
      <td nowrap="true"><%=getLocString("network.ussdOpenDestRef")%>:</td>
      <td><input class=txt name=defaultUssdOpenDestRef id=defaultUssdOpenDestRef value="<%=bean.getDefaultUssdOpenDestRef() == null ? "" : bean.getDefaultUssdOpenDestRef()%>"></td>
    </tr>
  </table>
</div>

<div class=content>
  <div class=page_subtitle><%=getLocString("network_profiles.networks")%></div>
  <script>
    function setSort(sorting) {
      if (sorting == "<%=bean.getSort()%>")
        opForm.sort.value = "-<%=bean.getSort()%>";
      else
        opForm.sort.value = sorting;
      opForm.submit();
      return false;
    }
  </script>
  <table class=list>
    <col width="2%">
    <col width="50%">
    <col width="24%">
    <col width="24%">
    <tr>
      <th><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
      <th>
        <a href="javascript:setSort('name')" <%=bean.getSort().endsWith("name") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
           title="<%=getLocString("common.sortmodes.name")%>"><%=getLocString("common.sortmodes.name")%></a>
      </th>
      <th>
        <a href="javascript:setSort('abonentStatusMethod')" <%=bean.getSort().endsWith("abonentStatusMethod") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
           title="<%=getLocString("common.sortmodes.abonentStatusMethod")%>"><%=getLocString("common.sortmodes.abonentStatusMethod")%></a>
      </th>
      <th>
        <a href="javascript:setSort('ussdOpenDestRef')" <%=bean.getSort().endsWith("ussdOpenDestRef") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
           title="<%=getLocString("common.sortmodes.ussdOpenDestRef")%>"><%=getLocString("common.sortmodes.ussdOpenDestRef")%></a>
      </th>
    </tr>
    <%int posIdx=0;%>
    <%for (int i=0; i<bean.getRows().size(); i++) {%>
      <% NetworkProfileDataItem item = ((NetworkProfileDataItem)bean.getRows().get(i)); %>
      <% String name = item.getName();%>
      <tr class=row<%=(posIdx++) % 2%>>
        <td>
          <input class=check type=checkbox name=checked value="<%=name%>" <%=bean.isProfileChecked(name) ? "checked" : ""%>>
        </td>
        <td><a href="javascript:editSomething('<%=StringEncoderDecoder.encode(name)%>')" title="Edit network"><%=StringEncoderDecoder.encode(name)%></a></td>
        <td><%=item.getAbonentStatusMethod()%></td>
        <td><%=item.getUssdOpenDestRef()%></td>
      </tr>
    <%}%>
  </table>
  <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbAdd", "network_profiles.add", "network_profiles.add");
  page_menu_button(session, out, "mbDelete", "network_profiles.delete", "network_profiles.delete");
  page_menu_space(out);
  page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.save");
  page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset");
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>