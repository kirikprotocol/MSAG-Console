<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.BlackList" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
  <input type=hidden name=initiated value=true>

  <table class=properties_list cellspacing=0>
    <col width="1%">
    <col width="99%">
    <% int rowN = 0;%>
    <tr class=row<%=rowN++&1%>>
      <th>address</th>
      <td><input class=txt name=msisdn value="<%=StringEncoderDecoder.encode(bean.getMsisdn())%>"></td>
    </tr>
  </table>
  <br>
  <div class=query_result>
    <%if (bean.isInitiated()) {%>
      <%=bean.getMsisdn()%><%=bean.isFound() ? " is in black list" : " is not in black list"%>
    <%}%>
  </div>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbLookup", "common.buttons.lookup", "infosme.hint.lookup");
  if (bean.isInitiated() && bean.isFound())
    page_menu_button(session, out, "mbDelete",   "common.buttons.delete",  "infosme.hint.delete");
  else if (bean.isInitiated() && !bean.isFound())
    page_menu_button(session, out, "mbAdd", "common.buttons.add", "infosme.hint.add");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>