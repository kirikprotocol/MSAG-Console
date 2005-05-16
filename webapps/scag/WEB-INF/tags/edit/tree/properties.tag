<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 tag body-content="scriptless" %><%@
 attribute name="noColoredLines" required="false"%><%@
 attribute name="noHeaders" required="false"%>
<c:set var="SMPPGW_EDIT_TREE_noColoredLines" value="${noColoredLines}" scope="request"/>
<c:set var="SMPPGW_EDIT_TREE_noHeaders" value="${noHeaders}" scope="request"/>
<table class=properties_list cellspacing=0 cellspadding=0 style="width:99%">
<c:if test="${!SMPPGW_EDIT_TREE_noHeaders}">
  <col width="15%">
  <col width="85%">
</c:if>
<c:set var="prop_rowN" value="0" scope="request"/>
<jsp:doBody/>
</table>