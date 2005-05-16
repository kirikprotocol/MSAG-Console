<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title" required="true"%><%@
 attribute name="name"  required="true"%><%@
 attribute name="value" required="true"%><sm-ep:property title="${title}">
<input class=txt type="text" name="${fn:escapeXml(name)}" value="${fn:escapeXml(value)}" readonly></sm-ep:property>