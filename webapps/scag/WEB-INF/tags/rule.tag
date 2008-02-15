<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<%@taglib uri="/scag/func" prefix="smf"%>
<%@
 taglib prefix="sm-pm" tagdir="/WEB-INF/tags/page_menu"%>
<%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %>
<%@tag body-content="empty"%>
<%@attribute name="tname" required="true"%>
<%@attribute name="ttitle" required="true"%>
<%@attribute name="exists" required="false"%>
<%@attribute name="locked" required="false"%>

  <sm-ep:property title = "${ttitle}">
    <span style="display:none">
      <input style="cursor:pointer" type=submit  name="editRule${tname}" onClick="return openJedit('${smf:getComplexRuleId(bean.id,tname)}','edit'); " value="<fmt:message>service.edit.rule.edit</fmt:message>">
      <input style="cursor:pointer" type=submit  name="deleteRule${tname}" value="<fmt:message>service.edit.rule.delete</fmt:message>">
    </span>
    <span style="display:none">
      <input style="cursor:pointer" type=submit onClick="return openJedit('${smf:getComplexRuleId(bean.id,tname)}','add')" value="<fmt:message>service.edit.rule.add</fmt:message>">
    </span>
    <span style="display:inline">
        <input style="cursor:pointer" type=submit  name="unlockRule${tname}" onlock="onlock();" value="<fmt:message>service.edit.rule.unlock</fmt:message>">
    </span>
  </sm-ep:property>