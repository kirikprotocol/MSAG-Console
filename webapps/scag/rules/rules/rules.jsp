<%@ page import="ru.sibinco.scag.beans.rules.Rules"%>
 <%--
  Created by IntelliJ IDEA.
  User: dym
  Date: 04.10.2006
  Time: 13:43:49
  To change this template use File | Settings | File Templates.
--%>
<%@ include file="/WEB-INF/inc/header.jspf" %>
<%--c:out value="${param.id}"/--%>
<sm:bean className="ru.sibinco.scag.beans.rules.Rules"/>
<html><head></head>
<script>
  var rulesState = new Array();
//  window.opener.test = "BINGO!!!";
//alert("rules");
  function populateArray() {
    rulesState[0]={exists:${bean.smppRuleState.exists},locked:${bean.smppRuleState.locked}};
    rulesState[1]={exists:${bean.httpRuleState.exists},locked:${bean.httpRuleState.locked}};
    rulesState[2]={exists:${bean.mmsRuleState.exists},locked:${bean.mmsRuleState.locked}};
  }
</script>
<body onload="populateArray()">
<input type="hidden" name="smppRuleStateE" id="smppRuleStateE" value="${bean.smppRuleState.exists}"/>
<input type="hidden" name="smppRuleStateL" id="smppRuleStateL" value="${bean.smppRuleState.locked}"/>
<input type="hidden" name="httpRuleStateE" id="httpRuleStateE" value="${bean.httpRuleState.exists}"/>
<input type="hidden" name="httpRuleStateL" id="httpRuleStateL" value="${bean.httpRuleState.locked}"/>
<input type="hidden" name="mmsRuleStateE"  id="mmsRuleStateE"  value="${bean.mmsRuleState.exists}"/>
<input type="hidden" name="mmsRuleStateL"  id="mmsRuleStateL"  value="${bean.mmsRuleState.locked}"/>

</body>
</html>