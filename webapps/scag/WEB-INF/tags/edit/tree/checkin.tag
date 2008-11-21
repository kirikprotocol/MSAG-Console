<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/scag/func"%><%@
 taglib prefix="sm-et" tagdir="/WEB-INF/tags/edit/tree" %><%@
 tag body-content="empty" %><%@
 attribute name="title"       required="false"%><%@
 attribute name="name"        required="true"%>
 <%@attribute name="onchanged" required="false"%>

<c:set var="param_name" value="${smf:concat(COLLAPSING_TREE_PREFIX, name)}"/>

<sm-et:property title="${!empty title ? title : ''}">
    <input class=check type=checkbox name="collapsing_tree_param.${fn:escapeXml(param_name)}"
            id="id_collapsing_tree_param.${fn:escapeXml(param_name)}"
            <c:if test="${!empty onchanged}">onfocus="${onchanged}" </c:if>
            value="true"
            <c:if test="${bean.params[param_name]}">checked</c:if>>&nbsp;
            <c:if test="${empty title}" >
                <label for="id_collapsing_tree_param.${fn:escapeXml(param_name)}">${!empty title ? title : name}</label>
            </c:if>
    <input name="collapsing_tree_param.${fn:escapeXml(param_name)}" type="hidden" value=""/>
</sm-et:property>