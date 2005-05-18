<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/scag/func"%><%@
 tag body-content="scriptless" %><%@
 attribute name="title" required="true"%><%@
 attribute name="name" required="true"%><%@
 attribute name="opened" required="false"%>
<c:set var="COLLAPSING_TREE_PREFIX" value="${smf:concat(!empty COLLAPSING_TREE_PREFIX ? COLLAPSING_TREE_PREFIX : '', smf:concat(name, '.'))}" scope="request"/>
<dl class="collapsing_tree">
	<dt id="collapsing_tree_${title}" <c:choose><c:when test="${opened}">class="opened"</c:when><c:otherwise>class="closed"</c:otherwise></c:choose>>${title}</dt>
  <dd id="collapsing_tree_${title}_body" style="width:100%;<c:if test='${!opened}'>display:none;</c:if>">
    <jsp:doBody/>
  </dd>
</dl>
<c:set var="COLLAPSING_TREE_PREFIX" value="${COLLAPSING_TREE_PREFIX == smf:concat(name, '.') ? '' : fn:substring(COLLAPSING_TREE_PREFIX, 0, fn:length(COLLAPSING_TREE_PREFIX) - (fn:length(name)+1))}" scope="request"/>
