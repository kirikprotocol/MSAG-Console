<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/scag/func"%><%@
 taglib prefix="sm-et" tagdir="/WEB-INF/tags/edit/tree" %><%@
 tag body-content="empty" %><%@
 attribute name="title"      required="false"%><%@
 attribute name="name"       required="true"%><%@
 attribute name="readonly"   required="false"%><%@
 attribute name="type"       required="false"%><%@
 attribute name="validation" required="false"%><c:set var="param_name" value="${smf:concat(COLLAPSING_TREE_PREFIX, name)}"
/><sm-et:property title="${!empty title ? title : name}"><input class=txt type="${empty type ? 'text' : type}" name="collapsing_tree_param.${fn:escapeXml(param_name)}" value="${fn:escapeXml(bean.params[param_name])}" <c:if test="${readonly}">readonly</c:if> <c:if test="${!empty validation}">validation="${validation}"  onkeyup="resetValidation(this)"</c:if>></sm-et:property>