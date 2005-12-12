function resetValidation(elem)
{
	elem.errorMessage = null;
	elem.runtimeStyle.color = elem.style.color;
	if(elem.tagName == "SELECT")
	{
		elem = elem.parentElement;
	}
	elem.runtimeStyle.borderColor = elem.style.borderColor;
/*	var errPointer = elem.nextSibling;
	if(errPointer != null)
	{
		errPointer.style.display = "none";
	}*/
}
function validateShowErrors(elem)
{
	if(elem.errorMessage == null) return;
	elem.title = elem.errorMessage;
	/*
	var errPointer = elem.nextSibling;
	if((errPointer == null) || (errPointer.className != "errPointer"))
	{
		errPointer = document.createElement("DIV");
		errPointer = elem.parentElement.appendChild(errPointer);
		errPointer.className="errPointer";
	}
	errPointer.style.display = "block";
	errPointer.runtimeStyle.right = elem.runtimeStyle.posRight;
	errPointer.runtimeStyle.top = elem.runtimeStyle.posTop;*/
}

function validationError(elem, txt)
{
	elem.errorMessage = txt;

	if (!elem.disabled)
	  elem.focus();

	elem.runtimeStyle.color = "#FF0000";
	if(elem.tagName == "SELECT")
	{
		elem = elem.parentElement;
	}
	elem.runtimeStyle.borderColor = "#FF0000";

	return false;
}

function validateField_port(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue >= 0xFFFF
		//? validationError(elem, "Port must be integer less than " + 0xFFFF)
		? validationError(elem, portErrorMsg + 0xFFFF)
		: true;
}

function validateField_mask(elem)
{
	if (elem.value == null || elem.value.length == 0)
	{
		return true;
	}
	//var pattern_header = "^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?";
	//var pattern1 = RegExp(pattern_header + "\\d{1,20}\\?{0,19}$");
	//var pattern2 = RegExp(pattern_header + "(\\d|\\?){1,20}$");
	var pattern1 = /^((\.[0-6]\.(0|1|3|4|6|8|9|10|14|18)\.)|(\+))?\d{0,20}\?{0,20}$/;
    var pattern2 = /^((\.[0-6]\.(0|1|3|4|6|8|9|10|14|18)\.)|(\+))?(\d|\?){1,20}$/;
	//var special_pattern_header = "^\\.5\\.0\\.";
	//var special_pattern1 = RegExp(special_pattern_header + "[ _\\-0-9A-Za-z]{1,20}\\?{0,19}$");
	//var special_pattern2 = RegExp(special_pattern_header + "([ _\\-0-9A-Za-z]|\\?){1,20}$");
    var special_pattern1 = /^\.5\.0\.[ _\-:0-9A-Za-z]{0,20}\?{0,20}$/;
    var special_pattern2 = /^\.5\.0\.([ _\-:0-9A-Za-z]|\?){1,20}$/;

	return elem.value == null
		|| (
			   (elem.value.match(        pattern1) == null || elem.value.match(        pattern2) == null)
			&& (elem.value.match(special_pattern1) == null || elem.value.match(special_pattern2) == null)
		   )
		//? validationError(elem, "Invalid mask")
		? validationError(elem, maskErrorMsg)
		: true;
}

function validateField_routeMask(elem)
{
	return validateField_mask(elem);
}

function validateField_select(elem)
{
	return (elem.selectedIndex == 0)
		//? validationError(elem, "Please, select something")
		? validationError(elem, selectErrorMsg)
		: true;
}

function validateField_priority(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue < 0  || intValue > (0x10000/2)
		//? validationError(elem, "Priority must be positive integer less than " + (0x10000/2))
		? validationError(elem, priorityErrorMsg + (0x10000/2))
		: true;
}

function validateField_route_serviceId(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue < 0
		//? validationError(elem, "Service ID must be positive integer")
		? validationError(elem, servIDErrorMsg)
		: true;
}

function validateField_nonEmpty(elem)
{
	return elem.value == null || elem.value.length == 0
		//? validationError(elem, "Must be not empty")
		? validationError(elem, nonEmptyErrorMsg)
		: true;
}

function validateField_language(elem)
{
	var result = false;
	for (var e in supportedLanguages) if (elem.value == supportedLanguages[e]) result = true;
	if (!result) elem.value = defaultLanguage;
	return result
		? true
		: validationError(elem, nonLanguageErrorMsg);
}

function validateField_email(elem)
{
	//var r = RegExp("(^).*\@.*\..*$");
	var r = /(^).*\@.*\..*$/
	return elem.value == null || elem.value.match(r) == null
		? validationError(elem, maskErrorMsg)
		: true;
}

function validateField_positive(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue <= 0
		? validationError(elem, valueErrorMsg)
		: true;
}

function validateField_int_range(elem)
{
	var intValue = elem.value/1;
	var intMin = elem.range_min == null ? (1/0) : (elem.range_min/1);
	var intMax = elem.range_max == null ? (1/0) : (elem.range_max/1);
	return isNaN(intValue) || ((!isNaN(intMin)) && intValue < intMin) || ((!isNaN(intMax)) && intValue > intMax)
		? validationError(elem, rangeValueErrorMsg_pre+elem.range_min+", "+elem.range_max+"]")
		: true;
}


function validateField_unsigned(elem)
{
	//var r = RegExp("^(\\s*)(\\d+)(\\s*)$");
	var r = /^(\s*)(\d+)(\s*)$/;
	return elem.value == null || elem.value.match(r) == null
		? validationError(elem, valueSignErrorMsg)
		: true;
}

function validateField_address(elem)
{
	if (elem.value == null || elem.value.length == 0)
	{
		return true;
	}
	//var r1 = RegExp("^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?\\d{1,20}$");
	//var r2 = RegExp("^\\.5\\.0\\.[ _\\-0-9A-Za-z]{1,20}$");
	var r1 = /^((\.[0-6]\.(0|1|3|4|6|8|9|10|14|18)\.)|(\+))?\d{1,20}$/
    var r2 = /^\.5\.0\.[ _\-:0-9A-Za-z]{1,20}$/;

	return elem.value == null || (elem.value.match(r1) == null && elem.value.match(r2) == null)
		? validationError(elem, addressErrorMsg)
		: true;
}

function validateField_address_prefix(elem)
{
	if (elem.value == null || elem.value.length == 0)
	{
		return true;
	}
	//var r1 = RegExp("^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?\\d{1,20}$");
	//var r2 = RegExp("^\\.5\\.0\\.[ _\\-0-9A-Za-z]{1,20}$");
	var r1 = /^((\.[0-6\*]\.(0|1|3|4|6|8|9|10|14|18|\*)\.)|(\+))?(\d|\*){1,20}$/
	var r2 = /^\.5\.0\.[ _\-:0-9A-Za-z\*]{1,20}$/;

	return elem.value == null || (elem.value.match(r1) == null && elem.value.match(r2) == null)
		? validationError(elem, addrPreErrorMsg)
		: true;
}

function validateField_id(elem)
{
	if (elem.value == null || elem.value.length == 0)
	{
		return true;
	}
	//var r1 = RegExp("^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?\\d{1,20}$");
	//var r2 = RegExp("^\\.5\\.0\\.[ _\\-0-9A-Za-z]{1,20}$");
	var r = /^[a-zA-Z_0-9]+$/

	return elem.value == null || elem.value.match(r) == null
		? validationError(elem, idErrorMsg)
		: true;
}

function validateField_reschedule(elem)
{
	if (elem.value == null || elem.value.length == 0)
	{
		return true;
	}
	var r = /^(\d+[s|m|h|d](\:\d+)?\,)*(\d+[s|m|h|d](\:(\d+|\*))?)$/

	return elem.value == null || elem.value.match(r) == null
		? validationError(elem, policyErrorMsg)
		: true;
}

var release_causes = [1,2,3,4,5,8,9,16,17,18,19,20,21,22,27,28,29,31,34,38,41,42,43,44,46,47,
                      50,53,55,57,58,62,63,65,69,70,79,83,84,87,88,90,91,95,97,99,102,103,110,111,121,127];
function validateField_release_cause(elem)
{
	if (elem.value == null || elem.value.length == 0)
		return validationError(elem, causeNullValueErrorMsg);

	var intValue = elem.value/1;
	if (isNaN(intValue) || intValue <= 0)
		return validationError(elem, causePositiveErrorMsg);

  for (var e in release_causes)
    if (intValue == release_causes[e]) return true;

  return validationError(elem, causeValueErrorMsg_pre+intValue+causeValueErrorMsg_post);
}

function validateField(elem)
{
	switch(elem.validation)
	{
		case "port":return validateField_port(elem);
		case "mask":return validateField_mask(elem);
		case "routeMask":return validateField_routeMask(elem);
		case "select":return validateField_select(elem);
		case "priority":return validateField_priority(elem);
		case "route_serviceId": return validateField_route_serviceId(elem);
		case "nonEmpty": return validateField_nonEmpty(elem);
		case "email": return validateField_email(elem);
		case "positive": return validateField_positive(elem);
		case "int_range": return validateField_int_range(elem);
		case "address": return validateField_address(elem);
		case "address_prefix": return validateField_address_prefix(elem);
		case "id" : return validateField_id(elem);
		case "unsigned": return validateField_unsigned(elem);
		case "reschedule": return validateField_reschedule(elem);
		case "release_cause": return validateField_release_cause(elem);
		case "language": return validateField_language(elem);
	}
	alert(unknownValidationTypeErrorMsg + ": "+elem.validation);
	return false;
}

function validateForm(frm)
{
	var result = true;
	var ts = frm.elements;
	for (var i=ts.length-1; i>=0; i--)
	{
		var elem = frm.elements(i);
		if (!elem.disabled)
		{
      var validationClass = elem.validation;
      if(validationClass != null)
      {
        var r = validateField(elem);
        validateShowErrors(elem);
        result = result && r;
      }
		}
	}
	return result;
}


function clickCancel()
{
	document.getElementById('jbutton').name = "mbCancel";
	opForm.submit();
	return false;
}

function clickClear()
{
	document.getElementById('jbutton').name = "mbClear";
	opForm.submit();
	return false;
}

function selectFirstTextInput()
{
	var inputs = document.body.getElementsByTagName("INPUT");
	if (inputs!=null)
	{
		for (i=0; i<inputs.length; i++)
		{
			try {
			if (inputs[i].type == "text"
					&& !inputs[i].readOnly
					&& !inputs[i].disabled
					&& inputs[i].currentStyle.display != "none"
				)
			{
				inputs[i].focus();
				return;
			}
			}
			catch (to_trash)
			{ // do nothing
			}
		}
	}
}

function findPosX(o)
{
	var x=0;
	if(o.offsetParent)
	{
		while(o.offsetParent)
		{
			x+=o.offsetLeft;
			o=o.offsetParent;
		}
	}
	else if(o.x)
		x+=o.x;
	return x;
}
function findPosY(o)
{
	var y=0;
	if(o.offsetParent)
	{
		while(o.offsetParent)
		{
			y+=o.offsetTop;
			o=o.offsetParent;
		}
	}
	else if(o.y)
		y+=o.y;
	return y;
}
function findPosWidth(o)
{
	var x=0;
	if(o.offsetParent)
	{
		while(o.offsetParent)
		{
			x+=o.offsetWidth;
			o=o.offsetParent;
		}
	}
	else if(o.width)
		x+=o.width;
	return x;
}
function findPosHeight(o)
{
	var x=0;
	if(o.offsetParent)
	{
		while(o.offsetParent)
		{
			x+=o.offsetHeight;
			o=o.offsetParent;
		}
	}
	else if(o.height)
		x+=o.height;
	return x;
}

function clickFilter()
{
	document.getElementById('jbutton').name = "mbFilter";
	opForm.submit();
	return false;
}

function noValidationSubmit(buttonElem, value, name)
{
  opForm.jbutton.value = value;
  opForm.jbutton.name = name
  opForm.submit();
  return false;
}

function editSomething(editObjectName)
{
	document.getElementById('jbutton').name = "mbEdit";
	document.getElementById('edit').value = editObjectName;
	opForm.submit();
	return false;
}

/** for navbar **/
function navigate(direction)
{
	document.getElementById('jbutton').name = direction;
	document.getElementById('jbutton').value = direction;
	opForm.submit();
	return false;
}

/** disable Delete button, if none of checkboxes checked **/
function checkCheckboxes(elem)
{
  var inputs = opForm.getElementsByTagName("input");
  var disabledElem = true;
  for (i = 0; i < inputs.length; i++)
  {
    var inp = inputs[i];
    if (inp.type == "checkbox")
      disabledElem &= !inp.checked;
  }
  elem.disabled = disabledElem;
  return true;
}

function checkCheckboxesForMbDeleteButton()
{
  return checkCheckboxes(document.getElementById('mbDelete'));
}

function byteToHexStr(bth)
{
	if (bth < 10)
		return String.fromCharCode(0x30 + bth);
	else
		return String.fromCharCode(0x41 + bth - 10);
}
function encodeHEX( str ) {
	var c = 0;
	var result = "";
	for (i = 0; i < str.length; i++) {
		result += byteToHexStr((str.charCodeAt(i)>>4)&0xF) + byteToHexStr((str.charCodeAt(i))&0xF);
	}
	return result;
}
function removeRow(tbl, rowId)
{
	var rowElem = tbl.rows(rowId);
	tbl.deleteRow(rowElem.rowIndex);
}

function findChildById(elem, id) {
  var childs = elem.childNodes;
  for ( var i = 0; i < childs.length; i++ ) {
    var child = childs.item(i);
    try {
      if( child.id == id ) return child;
    } catch (Exception) {
    }
    var res = findChildById( child, id );
    if( res != null ) return res;
  }
  return null;
}

function clickSubmit( name, value )
{
	if (opForm.onsubmit() == false)
		return false;
	opForm.jbutton.value = value;
	opForm.jbutton.name = name
	opForm.submit();
	return false;
}
