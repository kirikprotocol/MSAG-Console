<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title"      required="true"%><%@
 attribute name="name"       required="true"%><%@
 attribute name="readonly"   required="false"%><%@
 attribute name="validation" required="false"%><sm-ep:property title="${title}"><textarea name="${fn:escapeXml(name)}" <c:if test="${readonly}">readonly</c:if> <c:if test="${!empty validation}">validation="${validation}"  onkeyup="resetValidation(this)"</c:if>>${fn:escapeXml(bean[name])}</textarea></sm-ep:property>