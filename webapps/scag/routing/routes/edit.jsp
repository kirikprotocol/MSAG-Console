<%@include file="/WEB-INF/inc/header.jspf"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">routes.add.title</c:when>
      <c:otherwise>routes.edit.title</c:otherwise>
    </c:choose>
  </jsp:attribute>

  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="routes.edit.item.mbsave.value" title="routes.edit.item.mbsave.title"/>
      <sm-pm:item name="mbCancel" value="routes.edit.item.mbcancel.value" title="routes.edit.item.mbcancel.title" onclick="clickCancel()"/><%--onclick="history.back();"/>--%>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
  <script>
  var global_counter = 0;
  function removeRow(tblName, rowId){
      table = document.getElementById( tblName );
      var rowElem = table.rows(rowId);
      table.deleteRow(rowElem.rowIndex);
  }

  function removeSrcSubj(rowId){
      var selectElem = opForm.all.srcSubjSelect;
      var tbl = opForm.all.sources_table;
      var rowElem = tbl.rows(rowId);
      var subjValue = rowElem.all.subjSrc.value;
      var oOption = document.createElement("OPTION");
      selectElem.options.add(oOption);
      oOption.innerText = subjValue;
      oOption.value = subjValue;
      selectElem.disabled = false;
      tbl.deleteRow(rowElem.rowIndex);
  }

  function addSourceMask(valueElem) {
      if (validateField(valueElem)) {
          var tbl = opForm.all.sources_table;
          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length + 1) & 1);
          newRow.id = "srcRow_" + (global_counter++);
          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/mask.gif">';
          newRow.appendChild(newCell);
          newCell = document.createElement("td");
          newCell.innerHTML = valueElem.value + '<input type=hidden name=srcMasks value="' + valueElem.value + '">';
          newRow.appendChild(newCell);
          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.sources_table, \'' + newRow.id + '\')" style="cursor: hand;">';
          newRow.appendChild(newCell);
          valueElem.value = "";
          valueElem.focus();
          return true;
      } else
          return false;
  }

  function addSourceSubj() {
      var selectElem = opForm.all.srcSubjSelect;
      if (selectElem.options.length > 0) {
          var subjValue = selectElem.options[selectElem.selectedIndex].value;
          var tbl = opForm.all.sources_table;
          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length + 1) & 1);
          newRow.id = "srcRow_" + (global_counter++);
          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/subject.gif">';
          newRow.appendChild(newCell);
          newCell = document.createElement("td");
          newCell.innerHTML = subjValue + '<input id=subjSrc type=hidden name=srcSubjs value="' + subjValue + '">';
          newRow.appendChild(newCell);
          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeSrcSubj(\'' + newRow.id + '\');" style="cursor: hand;">';
          newRow.appendChild(newCell);
          selectElem.options[selectElem.selectedIndex] = null;
          selectElem.focus();
          if (selectElem.options.length == 0)
              selectElem.disabled = true;
      }
  }

  function srcSmeIdChanged()
  {
      var rowDisabled = opForm.all.srcSmeId.options[opForm.all.srcSmeId.selectedIndex] == null || opForm.all.srcSmeId.options[opForm.all.srcSmeId.selectedIndex].value != 'MAP_PROXY';
      opForm.all.forwardTo_row.disabled = rowDisabled;
      opForm.all.forwardTo.disabled = rowDisabled;
  }



  var smesSelectText = "<select name=fake_dst_mask_sme_ id=newSmesSelect><c:forEach items="${bean.allSmes}" var="i"><option value=\"${fn:escapeXml(i)}\" id=\"option_${fn:escapeXml(i)}\">${fn:escapeXml(i)}</option></c:forEach></select>";

  function addDestMask() {
      if (validateField(opForm.all.newDstMask)) {
          var mask = opForm.all.newDstMask.value;
          var smeSelect = opForm.all.newDstMaskSme;
          var sme = smeSelect.options[smeSelect.selectedIndex].value;
          var tbl = opForm.all.destinations_table;

          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length + 1) & 1);
          newRow.id = "srcRow_" + (global_counter++);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/mask.gif">';
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = mask + '<input type=hidden name=dstMasks value="' + mask + '">';
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = smesSelectText;
          newSelect = newCell.all.newSmesSelect;
          newSelect.name = "dst_mask_sme_" + mask;
          newSelect.all["option_" + sme].selected = true;
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.destinations_table, \'' + newRow.id + '\')" style="cursor: hand;">';
          newRow.appendChild(newCell);
          opForm.all.newDstMask.value = "";
          opForm.all.newDstMask.focus();
          return true;
      } else
          return false;
  }
  function selectDefaultSme() {
      var selectElem = opForm.all.dstSubjSelect;
      var smeSelectElem = opForm.all.dstSubjSmeSelect;
      if (selectElem.options.length > 0) {
          var defaultSme = selectElem.options[selectElem.selectedIndex].defaultSme;
          if (defaultSme != null) {
              var optionElem = smeSelectElem.options(defaultSme, 0);
              if (optionElem != null)
                  optionElem.selected = true;
          }
      }
      return true;
  }
  function removeDestSubj(rowId)
  {
      var selectElem = opForm.all.dstSubjSelect;
      var tbl = opForm.all.destinations_table;
      var rowElem = tbl.rows(rowId);
      var subjValue = rowElem.all.subjDst.value;
      var subjDefaultSme = rowElem.all.subjDst.defaultSme;
      var oOption = document.createElement("OPTION");
      selectElem.options.add(oOption);
      oOption.innerText = subjValue;
      oOption.value = subjValue;
      oOption.defaultSme = subjDefaultSme;
      selectElem.disabled = false;
      tbl.deleteRow(rowElem.rowIndex);
      return selectDefaultSme();
  }
  function addDestSubj() {
      var selectElem = opForm.all.dstSubjSelect;
      if (selectElem.options.length > 0) {
          var subjValue = selectElem.options[selectElem.selectedIndex].value;
          var subjDefaultSme = selectElem.options[selectElem.selectedIndex].defaultSme;
          var tbl = opForm.all.destinations_table;
          var smeSelect = opForm.all.dstSubjSmeSelect;
          var sme = smeSelect.options[smeSelect.selectedIndex].value;
          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length + 1) & 1);
          newRow.id = "srcRow_" + (global_counter++);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/subject.gif">';
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = subjValue + '<input id=subjDst type=hidden name=checkedDestinations value="' + subjValue + '">';
          newCell.all.subjDst.defaultSme = subjDefaultSme;
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = smesSelectText;
          newSelect = newCell.all.newSmesSelect;
          newSelect.name = "dst_sme_" + subjValue;
          newSelect.all["option_" + sme].selected = true;
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeDestSubj(\'' + newRow.id + '\');" style="cursor: hand;">';
          newRow.appendChild(newCell);
          selectElem.options[selectElem.selectedIndex] = null;
          selectElem.focus();
          if (selectElem.options.length == 0)
              selectElem.disabled = true;

          return selectDefaultSme();
      }
  }
  </script>
    <c:set var="smes" value="${fn:join(bean.smeIds, ',')}"/>
    <table cellpadding="5" border=0>
      <col width="50%">
      <col width="50%">
      <tr>
          <td valign="top">
              <sm-ep:properties title="routes.edit.properties.route_info" noColoredLines="false">
                  <sm-ep:txt title="routes.edit.txt.id" name="id" maxlength="60" readonly="${!bean.add}" validation="nonEmpty"/>
                  <sm-ep:txt title="routes.edit.txt.notes" name="notes"/>
                  <sm-ep:list title="routes.edit.list.srcsmeid" name="srcSmeId"
                              values="${smes}" valueTitles="${smes}" onChange="srcSmeIdChanged();"/>
              </sm-ep:properties>
          </td>
          <td valign="top">
              <sm-ep:properties title="routes.edit.properties.route_options" noColoredLines="false" noHeaders="false">
                  <sm-ep:check title="routes.edit.check.active" name="active"/>
                  <sm-ep:check title="routes.edit.check.enabled" name="enabled"/>
                  <sm-ep:check title="routes.edit.check.archived" name="archived"/>
              </sm-ep:properties>
          </td>
      </tr>
      <tr>
        <td valign="top" colspan="2">
            <sm-ep:properties title="routes.edit.properties.route_slicing" noColoredLines="false">
                  <sm-ep:list title="routes.edit.list.slicing"  name="slicing"
                              values="${fn:join(bean.slicingTypes,',')}"
                              valueTitles="${fn:join(bean.slicingTypesTitles,',')}"/>
                  <sm-ep:list title="routes.edit.list.slicedRespPolicy" name="slicedRespPolicy"
                              values="${fn:join(bean.slicedRespPolicyTypes,',')}"
                              valueTitles="${fn:join(bean.slicedRespPolicyTypesTitles,',')}"/>
                  <sm-pm:space/>
              </sm-ep:properties>
        </td>
      </tr>
      <tr><td colspan="2"><hr></td></tr>
    </table>
    <table cellpadding="12">
      <col width="42">
      <col width="16">
      <col width="42">
      <tr>
        <td valign="top">
          <div class=page_subtitle><fmt:message>routes.edit.label.sources</fmt:message></div>
          <table cellspacing=0 cellpadding=0>
          <col width="50%" align=left>
          <col width="50%" align=right>
          <col width="0%" align=left>
          <tr valign="middle">
            <td><fmt:message>routes.edit.label.subject</fmt:message></td>
            <td><select id=srcSubjSelect class="txt" >
              <c:forEach items="${bean.allUncheckedSrcSubjects}" var="i">
                <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
              </c:forEach></select>
            </td>
            <td><img src="content/images/but_add.gif" onclick="addSourceSubj()" style="cursor:pointer;"></td>
          </tr>
          <tr>
            <td><fmt:message>routes.edit.label.mask</fmt:message></td>
            <td><input id=newSrcMask class=txt name=srcMasks validation="routeMaskNonEmpty" onkeyup="resetValidation(this)">
            </td>
            <td><img src="content/images/but_add.gif" onclick="addSourceMask(opForm.all.newSrcMask)" style="cursor:pointer;"></td>
          </tr>
          </table>

          <hr>

          <c:set var="rowN" value="0"/>
          <table id=sources_table class=properties_list cellspacing=0 cellpadding=0>
          <col width="1%">
          <col width="100%">
          <c:forEach items="${bean.srcSubjs}" var="i">
            <c:set var="esubj" value="${fn:escapeXml(i)}"/>
            <tr class="row${rowN%2}" id="subjRow_${esubj}">
              <td><img src="content/images/subject.gif"></td>
              <td>${esubj}<input id=subjSrc type=hidden name=srcSubjs value="${esubj}"></td>
              <td><img src="content/images/but_del.gif" onClick="removeSrcSubj('subjRow_${esubj}');" style="cursor:pointer;"></td>
            </tr>
            <c:set var="rowN" value="${rowN+1}"/>
          </c:forEach>
          <c:forEach items="${bean.srcMasks}" var="i">
            <c:set var="emask" value="${fn:escapeXml(i)}"/>
            <tr class="row${rowN%2}" id="maskRow_${emask}">
              <td><img src="content/images/mask.gif"></td>
              <td>${emask}<input type=hidden name=srcMasks value="${emask}"></td>
              <td><img src="content/images/but_del.gif" onClick="removeRow('sources_table', 'maskRow_${emask}')" style="cursor:pointer;"></td>
            </tr>
            <c:set var="rowN" value="${rowN+1}"/>
          </c:forEach>
          </table>
        </td>

        <td valign="top">
          <div class=page_subtitle><fmt:message>routes.edit.label.destinations</fmt:message></div>
          <table cellspacing=0 cellpadding=0>
          <col width="89%">
          <col width="10%" align=right>
          <col width="1%">
          <col width="0%">
          <tr>
            <td><fmt:message>routes.edit.label.subject</fmt:message></td>
            <td><select id=dstSubjSelect onchange="return selectDefaultSme();">
              <c:forEach items="${bean.allUncheckedDstSubjects}"  var="i">
                <option value="${fn:escapeXml(i.key)}" defaultSme="${fn:escapeXml(i.value)}">${fn:escapeXml(i.key)}</option>
              </c:forEach>
            </select></td>
            <td><select id=dstSubjSmeSelect>
              <c:forEach items="${bean.allSmes}" var="i">
                <option id="${fn:escapeXml(i)}" value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
              </c:forEach>
            </select></td>
            <td><img src="content/images/but_add.gif" onclick="addDestSubj()" style="cursor:pointer;"></td>
          </tr>
          <tr>
            <td><fmt:message>routes.edit.label.mask</fmt:message></td>
              <td><input id=newDstMask class=txt name=new_dstMask validation="routeMaskNonEmpty" onkeyup="resetValidation(this)">
              </td>
            <td><select name=new_dst_mask_sme_ id=newDstMaskSme>
              <c:forEach items="${bean.allSmes}" var="i">
                <option value="${fn:escapeXml(i)}" <c:if test="${i == bean.new_dst_mask_sme_}">selected</c:if>>${fn:escapeXml(i)}</option>
              </c:forEach>
              </select>
            </td>
            <td><img src="content/images/but_add.gif" onclick="addDestMask()" style="cursor:pointer;"></td>
          </tr>
          </table>
          <hr>
          <table class=properties_list cellspacing=0 cellpadding=0 id=destinations_table>
          <col width="1%">
          <col width="99%">
          <col width="1%">
          <col width="1%">
          <c:set var="rowN" value="0"/>
          <c:forEach items="${bean.dstSubjPairs}" var="i">
            <tr class="row${rowN%2}" id="subjRow_${fn:escapeXml(i.key)}">
              <td><img src="content/images/subject.gif"></td>
              <td>${fn:escapeXml(i.key)}<input id=subjDst type=hidden name=dstSubjs value="${fn:escapeXml(i.key)}" defaultSme="< %=StringEncoderDecoder.encode(bean.getDefaultSubjectSme(${fn:escapeXml(i.key)}))% >"></td>
              <td><select name="dst_sme_${fn:escapeXml(i.key)}">
                <c:forEach items="${bean.allSmes}" var="j">
                  <option value="${fn:escapeXml(j)}" <c:if test="${i.value == j}">selected</c:if>>${fn:escapeXml(j)}</option>
                </c:forEach>
                </select>
              </td>
              <td><img src="content/images/but_del.gif" onClick="removeDestSubj('subjRow_${fn:escapeXml(i.key)}');" style="cursor: pointer;"></td>
            </tr>
            <c:set var="rowN" value="${rowN+1}"/>
          </c:forEach>
          <c:forEach items="${bean.dstMaskPairs}" var="i">
            <tr class="row${rowN%2}" id="maskRow_${fn:escapeXml(i.key)}">
              <td><img src="content/images/mask.gif"></td>
              <td>${fn:escapeXml(i.key)}<input type=hidden name=dstMasks value="${fn:escapeXml(i.key)}"></td>
              <td><select name="dst_mask_sme_${fn:escapeXml(i.key)}" onkeyup="resetValidation(this)">
                <c:forEach items="${bean.allSmes}" var="j">
                  <option value="${fn:escapeXml(j)}" <c:if test="${i.value == j}">selected</c:if>>${fn:escapeXml(j)}</option>
                </c:forEach>
                </select>
              </td>
              <td><img src="content/images/but_del.gif" onClick="removeRow('destinations_table', 'maskRow_${fn:escapeXml(i.key)}')" style="cursor: pointer;"></td>
            </tr>
            <c:set var="rowN" value="${rowN+1}"/>
          </c:forEach>
          </table>
        </td>
      </tr>
    </table>
    <script>selectDefaultSme()</script>
  </jsp:body>
</sm:page>