<%@ page import="ru.novosoft.smsc.jsp.util.tables.impl.regions.RegionsDataItem,
                 ru.novosoft.smsc.jsp.smsc.region.RegionsBean,
                 ru.novosoft.smsc.jsp.smsc.region.RegionsBean"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.smsc.region.RegionsBean" />
<jsp:setProperty name="bean" property="*"/>

<%
  TITLE=getLocString("regions.title");
	MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(request);
  switch(beanResult)
	{
		case RegionsBean.RESULT_EDIT:
      response.sendRedirect("regionEdit.jsp?region=" + bean.getEdit());
      return;
    case RegionsBean.RESULT_ADD:
      response.sendRedirect("regionEdit.jsp");
      return;
		default:
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<div class=content>
  <div class=page_subtitle><%=getLocString("regions.default")%></div>
  <input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
  <input type=hidden name=sort id=sort value="<%=bean.getSort()%>">
  <input type=hidden name=edit id=edit>
  <table>
    <col width="1%">
    <col width="49%">
    <tr>
      <td><%=getLocString("regions.bandwidth")%>:</td>
      <td><input class=txt name=defaultBandwidth id=defaultBandwidth value="<%=bean.getDefaultBandwidth() == null ? "" : bean.getDefaultBandwidth()%>"></td>
    </tr>
    <tr>
      <td><%=getLocString("regions.email")%>:</td>
      <td><input class=txt name=defaultEmail id=defaultEmail value="<%=bean.getDefaultEmail() == null ? "" : bean.getDefaultEmail()%>"></td>
    </tr>
  </table>
</div>

<div class=content>
  <div class=page_subtitle><%=getLocString("regions.regions")%></div>
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
    <col width="1%">
    <col width="70%">
    <col width="29%">
    <tr>
      <th><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
      <th>
        <a href="javascript:setSort('name')" <%=bean.getSort().endsWith("name") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
           title="<%=getLocString("common.sortmodes.name")%>"><%=getLocString("common.sortmodes.name")%></a>
      </th>
      <th>
        <a href="javascript:setSort('bandWidth')" <%=bean.getSort().endsWith("bandWidth") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
           title="<%=getLocString("common.sortmodes.bandWidth")%>"><%=getLocString("common.sortmodes.bandWidth")%></a>
      </th>
    </tr>
    <%int posIdx=0;%>
    <%for (int i=0; i<bean.getRows().size(); i++) {%>
      <% RegionsDataItem item = ((RegionsDataItem)bean.getRows().get(i)); %>
      <% String name = item.getName();%>
      <% int id = item.getId();%>
      <tr class=row<%=(posIdx++) % 2%>>
        <td>
          <input class=check type=checkbox name=checked value="<%=id%>" <%=bean.isRegionChecked(String.valueOf(id)) ? "checked" : ""%>>
        </td>
        <td><a href="javascript:editSomething('<%=StringEncoderDecoder.encode(String.valueOf(id))%>')" title="Edit region"><%=StringEncoderDecoder.encode(name)%></a></td>
        <td><%=item.getBandWidth()%></td>
      </tr>
    <%}%>
  </table>
  <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbAdd", "regions.add.region", "regions.add.region");
  page_menu_button(session, out, "mbDelete", "regions.delete.regions", "regions.delete.regions");
  page_menu_space(out);
  page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.save");
  page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset");
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>