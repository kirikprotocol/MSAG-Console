<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.RoutesFilter
                 " %>
<%@ page import="java.util.*"%>
<%@ taglib uri="http://jakarta.apache.org/taglibs/input-1.0" prefix="input" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.RoutesFilter"/>
<jsp:setProperty name="bean" property="*"/>

<%
    TITLE = getLocString("routes.filterTitle");
    switch (bean.process(request)) {
        case RoutesFilter.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case RoutesFilter.RESULT_OK:
            break;
        case RoutesFilter.RESULT_ClEAR:
            response.sendRedirect("routesFilter.jsp");
            break;
        case RoutesFilter.RESULT_ERROR:

            break;
    }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
    MENU0_SELECTION = "MENU0_ROUTES";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbApply", "common.buttons.apply", "common.buttons.applyFilter");
    page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearFilter", "clickClear()");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelFilterEditing", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<div class=content>
<table class=properties_list cellspacing=0 cellspadding=0>
    <col width="1%">
    <col width="85%">
</table>
<%int rowN = 0;%>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>

<%--<table class=properties_list cellspacing=0 cellspadding=0>
<%
rowN = 0;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td nowrap colspan=2><input class=check id="src_chk_<%=encName%>" type=checkbox name=srcChks value="<%=encName%>" <%=bean.isSrcChecked(name) ? "checked" : ""%>>&nbsp;<label for="src_chk_<%=encName%>"><%=encName%></label></td>
</tr>
<%}
--%>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ destinations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%--<table class=properties_list cellspacing=0 cellspadding=0>
<%
rowN = 0;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td nowrap colspan=2><input class=check id="dst_chk_<%=encName%>" type=checkbox name=dstChks value="<%=encName%>" <%=bean.isDstChecked(name) ? "checked" : ""%>>&nbsp;<label for="dst_chk_<%=encName%>"><%=encName%></label></td>
</tr>
<%}
--%>
<%
    TreeMap osrc = new TreeMap();
    java.util.HashMap asrc = new HashMap();
    Collection srcs = bean.getAllSubjects();
    if (srcs.size() < 10)
        asrc.put("size", String.valueOf(srcs.size()));
    else
        asrc.put("size", "10");
    asrc.put("multiple", "1");
    for (Iterator i = srcs.iterator(); i.hasNext();) {
        String name = (String) i.next();
        String encName = StringEncoderDecoder.encode(name);
        osrc.put(encName, name);
    }

%>
<%
    TreeMap odst = new TreeMap();
    Collection dsts = bean.getAllSubjects();
    for (Iterator i = srcs.iterator(); i.hasNext();) {
        String name = (String) i.next();
        String encName = StringEncoderDecoder.encode(name);
        odst.put(encName, name);
    }
    HashMap adst = new HashMap();
    if (dsts.size() < 10)
        adst.put("size", String.valueOf(dsts.size()));
    else
        adst.put("size", "10");
    adst.put("multiple", "1");
%>
<input type=hidden name=initialized value=true>

<table width="100%" class=properties_list cellspacing=0 cellspadding=0>
  <col width="1%"/>
  <col width="49%"/>
  <col width="1%"/>
  <col width="49%"/>
  <tr>
    <td colspan="2"><div class=page_subtitle><%=getLocString("common.titles.general")%></div></td>
    <td colspan="2"><div class=page_subtitle><%=getLocString("common.titles.options")%></div></td>
  </tr>

  <tr class=row<%=(rowN++) & 1%>>
    <td align="left" nowrap><%=getLocString("common.util.Name")%>: &nbsp;</td>
    <td align="left"><input:text name="filteredName" default="<%=bean.getFilteredName()%>"/></td>
    <td nowrap align="left"><%=getLocString("common.util.Filter")%>&nbsp;</td>
    <td nowrap align="left">
      <select name="strict1" id="strict1">
        <option value="1" <%=bean.getStrict1()==1 ? "selected" : ""%>><%=getLocString("routes.softFilter")%></option>
        <option value="2" <%=bean.getStrict1()==2 ? "selected" : ""%>><%=getLocString("routes.halfSoftFilter")%></option>
        <option value="0" <%=bean.getStrict1()==0 ? "selected" : ""%>><%=getLocString("routes.hardFilter")%></option>
      </select>
  </tr>

  <tr class=row<%=(rowN++) & 1%>>
    <td align="left" nowrap><%=getLocString("common.util.Provider")%>: &nbsp;</td>
    <td align="left"><input:text name="queryProvider" default="<%=bean.getQueryProvider()%>"/></td>
    <td nowrap align="left"><%=getLocString("routes.showSourcesList")%>&nbsp;</td>
    <td nowrap align="left"><input id=options_show_src class=check type="checkbox" name=showSrc <%=bean.isShowSrc() ? "checked" : ""%>></td>
  </tr>

  <tr class=row<%=(rowN++) & 1%>>
    <td align="left" nowrap><%=getLocString("common.util.Category")%>: &nbsp;</td>
    <td align="left"><input:text name="queryCategory" default="<%=bean.getQueryCategory()%>"/></td>
    <td nowrap align="left"><%=getLocString("routes.showDestinationsList")%>&nbsp;</td>
    <td nowrap align="left"><input id=options_show_dst class=check type="checkbox" name=showDst <%=bean.isShowDst() ? "checked" : ""%>></td>
  </tr>
  <tr class=row<%=(rowN++) & 1%>>
    <td colspan=2><div class=page_subtitle><%=getLocString("common.titles.sources")%></div></td>
    <td colspan=2><div class=page_subtitle><%=getLocString("common.titles.destinations")%></div></td>
  </tr>
  <tr class=row<%=(rowN++) & 1%>>
    <td>Subjects</td>
    <td><input:select name="srcChks" bean="bean" attributes="<%= asrc %>" options="<%= osrc %>"/></td>
    <td>Subjects</td>
    <td><input:select name="dstChks" bean="bean" attributes="<%= adst %>" options="<%= odst %>"/></td>
  </tr>
  <tr class=row<%=(rowN++) & 1%>>
    <td>Masks</td>
    <td>
      <table cellspacing=0 cellspadding=0>
        <col width="70%">
        <col width="30%">
        <% for (int i = 0; i < bean.getSrcMasks().length; i++) { %>
        <tr>
          <td><input class=txtW name=srcMasks value="<%=bean.getSrcMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
          <td>&nbsp;</td>
        </tr>
        <%}%>
        <tr>
          <td><input class=txtW name=srcMasks validation="mask" onkeyup="resetValidation(this)"></td>
          <td align="left"><%addButton(out, "mbAdd", "Add", "routes.addMask2SourcesHint");%></td>
        </tr>
      </table>
    </td>
    <td>Masks</td>
    <td>
      <table cellspacing=0 cellspadding=0>
        <col width="70%">
        <col width="30%">
        <%for (int i = 0; i < bean.getDstMasks().length; i++) { %>
        <tr>
          <td><input class=txtW name=dstMasks value="<%=bean.getDstMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
          <td>&nbsp;</td>
        </tr>
        <%}%>
        <tr>
          <td><input class=txtW name=dstMasks validation="mask" onkeyup="resetValidation(this)"></td>
          <td align="left"><%addButton(out, "mbAdd", "Add", "routes.addMask2DestHint");%></td>
        </tr>
      </table>
    </td>
  </tr>
  <tr class=row<%=(rowN++) & 1%>>
    <%
        TreeMap osme = new TreeMap();
        HashMap asme = new HashMap();
        TreeMap oasme = new TreeMap();
        Collection SMEs = bean.getAllSmes();
        List sme = (List) SMEs;
        for (int i = 0; i < sme.size(); i++) {
            String name = (String) sme.get(i);
            String encName = StringEncoderDecoder.encode(name);
            osme.put(encName, name);
        }
        if (sme.size() < 10)
            asme.put("size", String.valueOf(sme.size()));
        else
            asme.put("size", "10");
        asme.put("multiple", "1");
    %>
    <td>SMEs</td>
    <td><input:select name="srcSmeChks" bean="bean" attributes="<%= asme %>" options="<%= osme %>"/></td>
    <td>SMEs</td>
    <td><input:select name="dstSmeChks" bean="bean" attributes="<%= asme %>" options="<%= osme %>"/></td>
  </tr>
</table>

</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbApply", "common.buttons.apply", "common.buttons.applyFilter");
    page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearFilter", "clickClear()");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelFilterEditing", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>