<script>
function splitString(str)
{
	var arr = new Array();
	var buf = new String();
	for (i=0; i<str.length; i++)
	{
		switch (str.charAt(i))
		{
			case '\n':
			case '\r':
			case '\t': 
			case ' ': 
				if (buf.length > 0) {
				 arr.push(buf);
				 buf = new String();
				}
				break;
			default:
				buf += str.charAt(i);
		}
	}
	if (buf.length > 0) 
	{
		arr.push(buf);
		buf = new String();
	}
	return arr;
}

function checkMasks(masks)
{
	var arr = splitString(masks);
	var result = true;
	for (i=0; i<arr.length; i++) 
	{
		var r = /^\+{0,1}\d{1,21}(\*|\?{0,20})$/;
		if (arr[i].match(r) == null) 
		{
			alert("mask \"" + arr[i] + "\" not valid");
			result = false;
		} 
	}
	return result;
}

function checkName(nm)
{
	if (nm == null || nm.length == 0) 
	{
		alert("Name cannot be empty");
		return false;
	}
	else
		return true;
}
</script>