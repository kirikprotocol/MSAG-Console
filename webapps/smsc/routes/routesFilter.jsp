<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.RoutesFilter,
                 ru.novosoft.smsc.jsp.util.tables.impl.route.RouteFilter,
                 java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder
                 "%>
<%@ taglib uri="http://jakarta.apache.org/taglibs/input-1.0" prefix="input" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.RoutesFilter"/>
<jsp:setProperty name="bean" property="*"/>

<%
TITLE = "Filter routes";
switch(bean.process(request))
{
	case RoutesFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case RoutesFilter.RESULT_OK:

		break;
	case RoutesFilter.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ROUTES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "clickCancel()");
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
HashMap asrc = new HashMap();
Collection srcs=bean.getAllSubjects();
 if (srcs.size()<10)
  asrc.put("size",String.valueOf(srcs.size()));
  else  asrc.put("size","10");
  asrc.put("multiple","1");
  for (Iterator i = srcs.iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
  osrc.put( encName,name);
}

%>
<%
TreeMap odst = new TreeMap();
Collection dsts=bean.getAllSubjects();
for (Iterator i = srcs.iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
  odst.put( encName,name);
}
  HashMap adst = new HashMap();
  if (dsts.size()<10)
  adst.put("size",String.valueOf(dsts.size()));
  else  adst.put("size","10");
  adst.put("multiple","1");
%>
<table  with="100%">
  <tr>
   <td with="45%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
    <div class=page_subtitle>Sources</div>
    <table  with="100%">
     <tr>
      <td><input:select name="srcChks" bean="bean"
        attributes="<%= asrc %>" options="<%= osrc %>"  /></td></tr>
    </table>

   </td>
   <td width=10%>
&nbsp;
</td>
   <td with="45%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ destinations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>

     <div class=page_subtitle>Destinations</div>


     <table  with="100%">
      <tr>
       <td><input:select name="dstChks"  bean="bean"
       attributes="<%= adst %>" options="<%= odst %>"  /></td></tr>
     </table>
    </td>
  </tr>
</table>



<table class=properties_list cellspacing=0 cellspadding=0>
<tr>
<td width=45%>
<table class=properties_list cellspacing=0 cellspadding=0>
<%
for (int i=0; i<bean.getSrcMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<td colspan=1><input class=txtW name=srcMasks value="<%=bean.getSrcMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<td width=100%><input class=txtW name=srcMasks validation="mask" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new mask to sources filter");%></td>
</tr>
</table>
</td>
<td width=3%>
&nbsp;
</td>
<td width=52%>
<table class=properties_list cellspacing=0 cellspadding=0>
<%
for (int i=0; i<bean.getDstMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<td colspan=1><input class=txtW name=dstMasks value="<%=bean.getDstMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<td width=100%><input class=txtW name=dstMasks validation="mask" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new mask to destinations filter");%></td>
</tr>
</table>
</td>
</tr>
</table>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMEs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>SMEs</div>
<%
TreeMap osme = new TreeMap();
HashMap asme = new HashMap();
TreeMap oasme = new TreeMap();
Collection SMEs=bean.getAllSmes();
List sme = (List) SMEs;
for (int i=0; i<sme.size(); i++) {
 String name = (String)sme.get(i) ;
String encName = StringEncoderDecoder.encode(name);
    osme.put( encName,name);
}
  if (sme.size()<10)
  asme.put("size",String.valueOf(sme.size()));
  else  asme.put("size","10");
  asme.put("multiple","1");
%>
<%--
<table  with="100%">
<tr>
<td>

<%
  if (sme.size()<10) {
%>
 <select name=smeChks bean=bean multiple=true size=<%=sme.size()%>>
<%
      } else {
%>
<select name=smeChks bean=bean multiple=true size=10>
<%
  }
  // Iterator i = bean.getAllSmes().iterator();
  for (Iterator i = bean.getAllSmes().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);

      if (bean.isSmeChecked(name) ) {
%>
         <option value=<%=name%> selected><%=name%></option>
<%
      } else {
%>
         <option value=<%=name%>><%=name%></option>
<%
      }
   }
%>
 </select>
</td></tr>
</table>
--%>
<table  with="100%">
<tr>
<td><input:select name="smeChks" bean="bean"
    attributes="<%= asme %>"  options="<%= osme %>" />
</td></tr>
</table>
<%--
<table class=properties_list cellspacing=0 cellspadding=0>
<%
rowN = 0;
for (Iterator i = bean.getAllSmes().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td nowrap><input class=check id="sme_<%=encName%>" type=checkbox name=smeChks value="<%=encName%>" <%=bean.isSmeChecked(name) ? "checked" : ""%>>&nbsp;<label for="sme_<%=encName%>"><%=encName%></label></td>
</tr>
<%}%>
</table>
--%>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Options</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<%rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<td nowrap><input id=options_strict_filter class=check type="checkbox" name=strict <%=bean.isStrict() ? "checked" : ""%>>&nbsp;<label for=options_strict_filter>Strict Filter</label></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<td nowrap><input id=options_show_src class=check type="checkbox" name=showSrc <%=bean.isShowSrc() ? "checked" : ""%>>&nbsp;<label for=options_show_src>Show sources list</label></td>
</tr>
<tr class=rowLast>
	<td nowrap><input id=options_show_dst class=check type="checkbox" name=showDst <%=bean.isShowDst() ? "checked" : ""%>>&nbsp;<label for=options_show_dst>Show destinations list</label></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>