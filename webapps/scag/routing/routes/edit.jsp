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
//    alert( tblName + "|" + rowId );
      table = document.getElementById( tblName );
      var rowElem = table.rows[rowId];
      table.deleteRow(rowElem.rowIndex);
  }

  function addSourceMask(valueName) {
//    alert( "NAME =  " + valueName );
    var valueElem = getElementByIdUni( valueName );
      if (validateField(valueElem)) {
//          var tbl = opForm.all.sources_table;
          var tbl = getElementByIdUni("sources_table");
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
//          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.sources_table, \'' + newRow.id + '\')" style="cursor: hand;">';
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(\'sources_table\', \'' + newRow.id + '\')" style="cursor: pointer;">';
          newRow.appendChild(newCell);
          valueElem.value = "";
          valueElem.focus();
          return true;
      } else
          return false;
  }

  function addSourceSubj() {
//      var selectElem = opForm.all.srcSubjSelect;
      var selectElem = getElementByIdUni("srcSubjSelect");
      if (selectElem.options.length > 0) {
          var subjValue = selectElem.options[selectElem.selectedIndex].value;
//          var tbl = opForm.all.sources_table;
          var tbl = getElementByIdUni("sources_table");
          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length + 1) & 1);
          newRow.id = "srcRow_" + (global_counter++);
          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/subject.gif">';
          newRow.appendChild(newCell);
          newCell = document.createElement("td");
          newCell.innerHTML = subjValue + '<input id=subjSrc type=hidden name=srcSubjs value="' + subjValue + '">' + '<input id=srcSubj_' + subjValue + ' type=hidden name=srcSubj_' + subjValue + ' value="' + subjValue + '">';
          newRow.appendChild(newCell);
          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeSrcSubj(\'' + subjValue + '\',\'' + newRow.id + '\');" style="cursor: pointer;">';
          newRow.appendChild(newCell);
          selectElem.options[selectElem.selectedIndex] = null;
          selectElem.focus();
          if (selectElem.options.length == 0)
              selectElem.disabled = true;
      }
  }

  function removeSrcSubj(subj, rowId){
//      var selectElem = opForm.all.srcSubjSelect;
      var selectElem = getElementByIdUni("srcSubjSelect");
//      var tbl = opForm.all.sources_table;
      var tbl = getElementByIdUni("sources_table");

      var rowElem = tbl.rows[rowId];
//      var subjValue = rowElem.all.subjSrc.value;
//      var subjValue = rowElem.all.srcSubj.value;
      var subjValue = getElementByIdUni( 'srcSubj_' + subj ).value;
//      alert( "subjValue = " + subjValue);
      var oOption = document.createElement("OPTION");
      selectElem.options.add(oOption);
//      oOption.innerText = subjValue;
      oOption.text = subjValue;
      oOption.value = subjValue;
      selectElem.disabled = false;
      tbl.deleteRow(rowElem.rowIndex);
  }

  function srcSmeIdChanged()
  {
      var rowDisabled = opForm.all.srcSmeId.options[opForm.all.srcSmeId.selectedIndex] == null || opForm.all.srcSmeId.options[opForm.all.srcSmeId.selectedIndex].value != 'MAP_PROXY';
      opForm.all.forwardTo_row.disabled = rowDisabled;
      opForm.all.forwardTo.disabled = rowDisabled;
  }



  var smesSelectText = "<select name=fake_dst_mask_sme_ id=newSmesSelect><c:forEach items="${bean.allSmes}" var="i"><option value=\"${fn:escapeXml(i)}\" id=\"option_${fn:escapeXml(i)}\">${fn:escapeXml(i)}</option></c:forEach></select>";

  function addDestMask() {
      newDstMask = getElementByIdUni("newDstMask");
//      if (validateField(opForm.all.newDstMask)) {
      if (validateField(newDstMask)) {
//          var mask = opForm.all.newDstMask.value;
          var mask = newDstMask.value;
//          var smeSelect = opForm.all.newDstMaskSme;
          var smeSelect = getElementByIdUni("newDstMaskSme");
          if( smeSelect.selectedIndex < 0 ){
              alert( "Could't add mask with empty sme!" );
              return;
          }
          var sme = smeSelect.options[smeSelect.selectedIndex].value;
//          var tbl = opForm.all.destinations_table;
          var tbl = getElementByIdUni("destinations_table");

          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length + 1) & 1);
          newRow.id = "srcRow_" + (global_counter++);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/mask.gif">';
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = mask + '<input type=hidden name=dstMasks value="' + mask + '">';
          newRow.appendChild(newCell);

    var smeSelectHard = document.createElement("select");
    var i = 0;
    <c:forEach items="${bean.allSmes}" var="i">
        var opt = document.createElement("option");
        opt.value = "${fn:escapeXml(i)}";
        opt.text = "${fn:escapeXml(i)}";
        opt.id = "option_${fn:escapeXml(i)}";
        if( opt.valu == sme ){
            opt.selected = true;
        }
        smeSelectHard.options.add(opt);
        i++;
    </c:forEach>

          newCell = document.createElement("td");
          newCell.innerHTML = smesSelectText;
          newSelect = newCell.firstChild;
//          var newSelect = document.createElement("select");
//          newSelect = newSelect1;
          newSelect.name = "dst_mask_sme_" + mask;
          for(var i=0; i < newSelect.options.length; i=i+1){
            var op = newSelect.options[i];
            if(op.id == "option_" + sme){
//                alert(op.id + " | " + "option_" + sme);
                op.selected = true
            }
          }
//o          newSelect.all["option_" + sme].selected = true;
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(\'destinations_table\', \'' + newRow.id + '\')" style="cursor: pointer;">';
          newRow.appendChild(newCell);
//o          opForm.all.newDstMask.value = "";
//o          opForm.all.newDstMask.focus();
          newDstMask.value = "";
          newDstMask.focus();
          return true;
      } else
          return false;
  }
  function selectDefaultSme() {
//o      var selectElem = opForm.all.dstSubjSelect;
      var selectElem = getElementByIdUni("dstSubjSelect");
//o      var smeSelectElem = opForm.all.dstSubjSmeSelect;
      var smeSelectElem = getElementByIdUni("dstSubjSmeSelect");
      if (selectElem.options.length > 0) {
//          var defaultSme = selectElem.options[selectElem.selectedIndex].defaultSme;
          var defaultSme = selectElem.options[selectElem.selectedIndex].id;
//          alert("DSid=" + defaultSme);
          if (defaultSme != null) {
            setOptionSelected(smeSelectElem, 'id', defaultSme);

//o              var optionElem = smeSelectElem.options(defaultSme, 0);
//o              if (optionElem != null)
//o                  optionElem.selected = true;
          }
      }
      return true;
  }

    function setOptionSelected( selectElem, propertyName, value ){
        if(propertyName == 'id'){
//            alert("IN");
            for(var i=0; i<selectElem.options.length; i++){
                if(selectElem.options[i].id == value){
                    selectElem.options[i].selected = true;
                    return true;
                }
            }
        } else if(propertyName == 'text'){
            for(var i=0; i<selectElem.options.length; i++){
                if(selectElem.options[i].text == value){
                    selectElem.options[i].selected = true;
                    return true;
                }
            }
        } else if(propertyName == 'value'){
            for(var i=0; i<selectElem.options.length; i++){
                if(selectElem.options[i].value == value){
                    selectElem.options[i].selected = true;
                    return true;
                }
            }
       }
       return false;
    }

  function removeDestSubj(rowId, key)
  {
//o      var selectElem = opForm.all.dstSubjSelect;
      var selectElem = getElementByIdUni("dstSubjSelect");
//o      var tbl = opForm.all.destinations_table;
      var tbl = getElementByIdUni("destinations_table");
      var rowElem = tbl.rows[rowId];
//o      var subjValue = rowElem.all.subjDst.value;
//o      var subjDefaultSme = rowElem.all.subjDst.defaultSme;
      var subjValue = getElementByIdUni( "subjDst_" + key).value;
      var subjDefaultSme = getElementByIdUni( "defaultSme_" + key).value;

//      alert( subjValue + " | " + subjDefaultSme);
      var oOption = document.createElement("OPTION");
      selectElem.options.add(oOption);
//o      oOption.innerText = subjValue;
      oOption.text = subjValue;
      oOption.value = subjValue;
      oOption.defaultSme = subjDefaultSme;
      selectElem.disabled = false;
      tbl.deleteRow(rowElem.rowIndex);
      return selectDefaultSme();
  }
  function addDestSubj() {
//o      var selectElem = opForm.all.dstSubjSelect;
      var selectElem = getElementByIdUni("dstSubjSelect");
      if (selectElem.options.length > 0) {
          var subjValue = selectElem.options[selectElem.selectedIndex].value;
//          var subjDefaultSme = selectElem.options[selectElem.selectedIndex].defaultSme;
          var subjDefaultSme = selectElem.options[selectElem.selectedIndex].id;
//          alert( "subjValue=" + subjValue + " | subjDefaultSme=" + subjDefaultSme);
//o          var tbl = opForm.all.destinations_table;
          var tbl = getElementByIdUni("destinations_table");
//o          var smeSelect = opForm.all.dstSubjSmeSelect;
          var smeSelect = getElementByIdUni("dstSubjSmeSelect");
          var sme = smeSelect.options[smeSelect.selectedIndex].value;
          var newRow = tbl.insertRow(tbl.rows.length);
          newRow.className = "row" + ((tbl.rows.length + 1) & 1);
          newRow.id = "srcRow_" + (global_counter++);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/subject.gif">';
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
//        newCell.innerHTML = subjValue + '<input id=subjDst type=hidden name=checkedDestinations value="' + subjValue + '">';
          newCell.innerHTML = subjValue + '<input id=subjDst type=hidden name=checkedDestinations value="' + subjValue + '" defaultSme ="' + subjDefaultSme + '"> <input id="subjDst_' +  subjValue + '" type=hidden value="' + subjValue + '"> <input id="defaultSme_' +  subjValue + '" type=hidden value="' + subjDefaultSme + '">';
//          newCell.id = 'subjDst';
//o          newCell.all.subjDst.defaultSme = subjDefaultSme;
//          newCell.lastChild.defaultSme = subjDefaultSme;
//          alert( "LCC = " + newCell.lastChild.value + "," + newCell.lastChild.id );
          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = smesSelectText;
//o          newSelect = newCell.all.newSmesSelect;
          newSelect = newCell.firstChild;
          newSelect.name = "dst_sme_" + subjValue;
//o          newSelect.all["option_" + sme].selected = true;
          setOptionSelected(newSelect, 'id', "option_" + sme);

          newRow.appendChild(newCell);

          newCell = document.createElement("td");
          newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeDestSubj(\'' + newRow.id + '\',\'' + subjValue + '\');" style="cursor: pointer;">';
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
                  <sm-ep:txt title="routes.edit.txt.id" name="id" maxlength="50" readonly="${!bean.add}" validation="nonEmpty"/>
                  <sm-ep:txt title="routes.edit.txt.notes" name="notes"/>
                  <sm-ep:list title="routes.edit.list.srcsmeid" name="srcSmeId"
                              values="${smes}" valueTitles="${smes}" onChange="srcSmeIdChanged();"/>
              </sm-ep:properties>
          </td>
          <td valign="top">
              <sm-ep:properties title="routes.edit.properties.route_options" noColoredLines="false" noHeaders="false">
                  <sm-ep:check title="routes.edit.check.enabled" name="enabled"/>
                  <sm-ep:check title="routes.edit.check.transit" name="transit"/>
                  <sm-ep:check title="routes.edit.check.statistic_saa" name="saa"/>
                  <sm-ep:check title="routes.edit.check.hideSaaText" name="hideSaaText"/>
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
            <td><input id=newSrcMask class=txt name=srcMasks validation="routeMask" onkeyup="resetValidation(this)">
            </td>
            <td><img src="content/images/but_add.gif" onclick="addSourceMask('newSrcMask')" style="cursor:pointer;"></td>
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
              <td>${esubj}<input id=subjSrc type=hidden name=srcSubjs value="${esubj}">
                    <input type=hidden id="srcSubj_${esubj}" name="srcSubj_${esubj}" value="${esubj}">
              </td>
              <td><img src="content/images/but_del.gif" onClick="removeSrcSubj( '${esubj}', 'subjRow_${esubj}');" style="cursor:pointer;"></td>
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
                <option value="${fn:escapeXml(i.key)}" defaultSme="${fn:escapeXml(i.value)}" id="${fn:escapeXml(i.value)}">${fn:escapeXml(i.key)}</option>
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
              <td><input id=newDstMask class=txt name=new_dstMask validation="routeMask" onkeyup="resetValidation(this)">
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
              <td>${fn:escapeXml(i.key)}
                  <input id=subjDst type=hidden name=dstSubjs value="${fn:escapeXml(i.key)}" defaultSme="< %=StringEncoderDecoder.encode(bean.getDefaultSubjectSme(${fn:escapeXml(i.key)}))% >">
                  <input id="subjDst_${fn:escapeXml(i.key)}" type=hidden name=dstSubjs value="${fn:escapeXml(i.key)}" >
                  <input id="defaultSme_${fn:escapeXml(i.key)}" type=hidden value="< %=StringEncoderDecoder.encode(bean.getDefaultSubjectSme(${fn:escapeXml(i.key)}))% >">
              </td>
              <td><select name="dst_sme_${fn:escapeXml(i.key)}">
                <c:forEach items="${bean.allSmes}" var="j">
                  <option value="${fn:escapeXml(j)}" <c:if test="${i.value == j}">selected</c:if>>${fn:escapeXml(j)}</option>
                </c:forEach>
                </select>
              </td>
              <td><img src="content/images/but_del.gif" onClick="removeDestSubj('subjRow_${fn:escapeXml(i.key)}', '${fn:escapeXml(i.key)}');" style="cursor: pointer;"></td>
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
<%--last--%>