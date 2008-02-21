<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/scag/func"%><%@
 tag body-content="scriptless" %><%@
 attribute name="title" required="true"%><%@
 attribute name="name" required="true"%><%@
 attribute name="opened" required="false"%><%@
 attribute name="type" required="false"%><%@
 attribute name="tname" required="false"%><%@
 attribute name="width" required="false"%>
<c:set var="COLLAPSING_TREE_PREFIX" value="${smf:concat(!empty COLLAPSING_TREE_PREFIX ? COLLAPSING_TREE_PREFIX : '', smf:concat(name, '.'))}" scope="request"/>
<c:choose>
    <c:when test="${type == 'table'}">
        <table border=0>
            <tr class="">
              <td colaspan=2 <c:choose>
                                <c:when test="${opened}">class="collapsing_tree_opened"</c:when>
                                <c:otherwise>            class="closed"</c:otherwise>
                             </c:choose>
                             <c:choose>
                                <c:when test="${! empty tname}">onclick="closeProp('${tname}');"> <div class="collapsing_tree_closed" id="${tname}_div">${title}</div></c:when>
                                <c:otherwise>onclick="closeProp('${name}');"> <div class="collapsing_tree_closed" id="${name}_div">${title}</div></c:otherwise>
                             </c:choose>
              </td>
            </tr>
            <tr>
                <td><%--<table id="${name}" style="display:none"><tr><td>--%>
                    <jsp:doBody/> <%--        </td></tr></table>--%>
                </td>
            </tr>
        </table>
    </c:when>
    <c:otherwise>
        <dl class="collapsing_tree">
            <dt id="collapsing_tree_${title}"
                <c:choose>
                    <c:when test="${opened}">class="opened"</c:when>
                    <c:otherwise>class="closed"</c:otherwise></c:choose>>
                    ${title}
            </dt>
            <dd id="collapsing_tree_${title}_body" style="width:
                <c:choose>
                    <c:when test="${!empty width}">${width}</c:when>
                    <c:otherwise>100%</c:otherwise>
                </c:choose>;
                <c:if test='${!opened}'>display:none;</c:if>">
                <jsp:doBody/>
              </dd>
        </dl>
    </c:otherwise>
</c:choose>

<c:set var="COLLAPSING_TREE_PREFIX" value="${COLLAPSING_TREE_PREFIX == smf:concat(name, '.') ? '' : fn:substring(COLLAPSING_TREE_PREFIX, 0, fn:length(COLLAPSING_TREE_PREFIX) - (fn:length(name)+1))}" scope="request"/>
