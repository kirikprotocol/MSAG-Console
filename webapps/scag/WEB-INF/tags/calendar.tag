<%@
 taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 tag body-content="empty"%><%@
 attribute name="name" required="true" %><%@
 attribute name="value" required="false"%><%@
 attribute name="readonly"   required="false"%><%@
 attribute name="validation" required="false"%><input type="text" id="${fn:escapeXml(name)}" name="${fn:escapeXml(name)}" class=calendarField value="${!empty value ? value : fn:escapeXml(bean[name])}" maxlength=20 style="z-index:22;" <c:if test="${readonly}">readonly</c:if> <c:if test="${!empty validation}">validation="${validation}"  onkeyup="resetValidation(this)"</c:if>><button class=calendarButton type=button onclick="return showCalendar(${fn:escapeXml(name)}, false, true);">...</button>