<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="sm" tagdir="/WEB-INF/tags"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title"      required="false"%><%@
 attribute name="name"       required="true"%><%@
 attribute name="value" required="false"%><%@
 attribute name="readonly"   required="false"%><%@
 attribute name="validation" required="false"%><sm-ep:property title="${title}"><sm:calendar name="${name}" readonly="${readonly}" validation="${validation}" value="${value}"/></sm-ep:property>