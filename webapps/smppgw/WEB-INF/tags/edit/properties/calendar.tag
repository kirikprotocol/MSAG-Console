<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title"      required="true"%><%@
 attribute name="name"       required="true"%><%@
 attribute name="readonly"   required="false"%><%@
 attribute name="validation" required="false"%><sm-ep:property title="${title}"><sm:calendar name="${name}" readonly="${readonly}" validation="${validation}"/></sm-ep:property>