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
		? validationError(elem, "Port must be integer less than " + 0xFFFF)
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
    var special_pattern1 = /^\.5\.0\.[ _\-0-9A-Za-z]{0,20}\?{0,20}$/;
    var special_pattern2 = /^\.5\.0\.([ _\-0-9A-Za-z]|\?){1,20}$/;

	return elem.value == null
		|| (
			   (elem.value.match(        pattern1) == null || elem.value.match(        pattern2) == null)
			&& (elem.value.match(special_pattern1) == null || elem.value.match(special_pattern2) == null)
		   )
		? validationError(elem, "Invalid mask")
		: true;
}

function validateField_routeMask(elem)
{
	return validateField_mask(elem);
}

function validateField_select(elem)
{
	return (elem.selectedIndex == 0)
		? validationError(elem, "Please, select something")
		: true;
}

function validateField_priority(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue < 0  || intValue > (0x10000/2)
		? validationError(elem, "Priority must be positive integer less than " + (0x10000/2))
		: true;
}

function validateField_route_serviceId(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue < 0
		? validationError(elem, "Service ID must be positive integer")
		: true;
}

function validateField_nonEmpty(elem)
{
	return elem.value == null || elem.value.length == 0
		? validationError(elem, "Must be not empty")
		: true;
}

function validateField_email(elem)
{
	//var r = RegExp("(^).*\@.*\..*$");
	var r = /(^).*\@.*\..*$/
	return elem.value == null || elem.value.match(r) == null
		? validationError(elem, "Invalid mask")
		: true;
}

function validateField_positive(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue <= 0
		? validationError(elem, "value must be an integer and positive")
		: true;
}

function validateField_unsigned(elem)
{
	//var r = RegExp("^(\\s*)(\\d+)(\\s*)$");
	var r = /^(\s*)(\d+)(\s*)$/;
	return elem.value == null || elem.value.match(r) == null
		? validationError(elem, "value must be decimal without sign")
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
    var r2 = /^\.5\.0\.[ _\-0-9A-Za-z]{1,20}$/;

	return elem.value == null || (elem.value.match(r1) == null && elem.value.match(r2) == null)
		? validationError(elem, "Invalid address")
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
	var r2 = /^\.5\.0\.[ _\-0-9A-Za-z\*]{1,20}$/;

	return elem.value == null || (elem.value.match(r1) == null && elem.value.match(r2) == null)
		? validationError(elem, "Invalid address prefix")
		: true;
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
		case "address": return validateField_address(elem);
		case "address_prefix": return validateField_address_prefix(elem);
		case "unsigned": return validateField_unsigned(elem);
	}
	alert("unknown validation type:"+elem.validation);
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
	document.all.jbutton.name = "mbCancel";
	opForm.submit();
	return false;
}

function clickClear()
{
	document.all.jbutton.name = "mbClear";
	opForm.submit();
	return false;
}

function selectFirstTextInput()
{
	var inputs = document.all.tags("INPUT");
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
	document.all.jbutton.name = "mbFilter";
	opForm.submit();
	return false;
}
function noValidationSubmit(buttonElem)
{
  document.all.jbutton.value = buttonElem.jbuttonValue;
  document.all.jbutton.name  = buttonElem.jbuttonName;
  opForm.submit();
  return false;
}

function editSomething(editObjectName)
{
	opForm.all.jbutton.name = "mbEdit";
	opForm.all.edit.value = editObjectName;
	opForm.submit();
	return false;
}

/** for navbar **/
function navigate(direction)
{
	document.all.jbutton.name = direction;
	document.all.jbutton.value = direction;
	opForm.submit();
	return false;
}

/** disable Delete button, if none of checkboxes checked **/
function checkCheckboxesForMbDeleteButton()
{
  var inputs = opForm.getElementsByTagName("input");
  var disabledDelete = true;
  for (i = 0; i < inputs.length; i++)
  {
    var inp = inputs[i];
    if (inp.type == "checkbox")
      disabledDelete &= !inp.checked;
  }
  opForm.all.mbDelete.disabled = disabledDelete;
}
