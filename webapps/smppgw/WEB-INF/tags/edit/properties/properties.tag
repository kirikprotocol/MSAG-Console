<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 tag body-content="scriptless" %><%@
 attribute name="title" required="false"%><%@
 attribute name="noColoredLines" required="false"%><%@
 attribute name="noHeaders" required="false"%><%@
 attribute name="noEdit" required="false"%>
<c:set var="SMPPGW_EDIT_PROPERTIES_noColoredLines" value="${noColoredLines}" scope="request"/>
<c:set var="SMPPGW_EDIT_PROPERTIES_noHeaders" value="${noHeaders}" scope="request"/>
<c:if test="${empty noEdit || !noEdit}">
<input type="hidden" name="editId" value="${fn:escapeXml(bean.editId)}">
<input type="hidden" name="add" value="${fn:escapeXml(bean.add)}">
</c:if>
<div class=page_subtitle>${fn:escapeXml(title)}</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<c:if test="${!SMPPGW_EDIT_PROPERTIES_noHeaders}">
  <col width="15%">
  <col width="85%">
</c:if>
<c:set var="prop_rowN" value="0" scope="request"/>
<jsp:doBody/>
</table>