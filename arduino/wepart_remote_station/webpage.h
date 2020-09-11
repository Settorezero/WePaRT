//String header="HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"; // not required 
String doctype="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n";

// first part of html page: meta-tags and stylesheet
String p1a = "<html>\n<head>\n"
"<title>WePaRT by Giovanni Bernardo</title>\n"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\">\n";

String ref="<meta http-equiv=\"refresh\" content=\"5\">";

String p1b="<style type=\"text/css\">\n"
"body {background-color:#07070A;}\n"
".st {font-family:tahoma,arial; font-size:22pt; font-weight:bold; text-align:left; padding-left:10px; margin-top:8px; height:30pt; vertical-align:middle; display:block; border-radius:5px;}\n"
".ti {color:#FFFFFF; font-family:tahoma,arial; font-size:14pt; font-weight:bold; text-align:center; padding:8px; margin-top:2px; display:block;}\n"
".ti2 {color:#FFFFFF; font-family:tahoma,arial; font-size:12pt; font-weight:bold; text-align:center; padding:8px; margin-top:2px; display:block;}\n"
".s {color:#FFFFFF; text-decoration:none; font-family:tahoma,arial; font-size:8pt; font-weight:normal; text-align:center; padding:1px; display:block;}\n"
".sf {color:#FFFFFF; text-decoration:none; font-family:tahoma,arial; font-size:10pt; font-weight:normal; text-align:left; padding:1px; display:block;}\n"
".sfb {color:#000000; background-color:#FFFFFF; font-family:tahoma,arial; font-weigth:bold; font-size:10pt; text-align:left; padding:1px; display:block; border-radius:5px;}\n"
"a.l:hover, a.l:link, a.l:visited {color:#000000; background-color:#E4FF1A; text-decoration:none; font-family:tahoma,arial; font-size:12pt; height:14pt; vertical-align:middle; text-align:center; padding:1px; margin-top:5px; display:block; border-radius:5px;}\n"
"a.l33:hover, a.l33:link, a.l33:visited {color:#000000; background-color:#E4FF1A; text-decoration:none; font-family:tahoma,arial; font-size:11pt; height:14pt; vertical-align:middle; text-align:center; padding:1px; margin-top:5px; width:33%; float:right; border-radius:5px;}\n"
"</style>\n";

// second part of html page: ajax functions
String p2 = "<script>\n"
"\tsetInterval(function() {getSensors();}, 2000);\n"
"\tvar sensorValues=[];\n"
"\tfunction getSensors() {\n"
"\t\tvar xhttp=new XMLHttpRequest();\n"
"\t\txhttp.onreadystatechange=function() {\n"
"\t\t\tif (this.readyState==4 && this.status==200) {\n"
"\t\t\t\tsensorValues=this.responseText.split(\";\");\n"
"\t\t\t\tdocument.getElementById(\"TIMESTAMP\").innerHTML=sensorValues[1];\n"
"\t\t\t\tdocument.getElementById(\"TB\").innerHTML=sensorValues[2]+\"&deg;C\";\n"
"\t\t\t\tdocument.getElementById(\"TD\").innerHTML=sensorValues[3]+\"&deg;C\";\n"
"\t\t\t\tif (sensorValues[0]==\"12\"){"
"\t\t\t\t\tdocument.getElementById(\"HB\").innerHTML=sensorValues[4]+\"%\";\n"
"\t\t\t\t\tdocument.getElementById(\"HD\").innerHTML=sensorValues[5]+\"%\";\n"
"\t\t\t\t\tdocument.getElementById(\"P\").innerHTML=sensorValues[6]+\"mBar\";\n"
"\t\t\t\t\tdocument.getElementById(\"PM10\").innerHTML=sensorValues[7]+\"&micro;g/m<sup>3</sup>\";\n"
"\t\t\t\t\tdocument.getElementById(\"PM25\").innerHTML=sensorValues[8]+\"&micro;g/m<sup>3</sup>\";\n"
"\t\t\t\t\tdocument.getElementById(\"RSSI\").innerHTML=sensorValues[12];\n"
"\t\t\t\t}"
"\t\t\t\telse{"
"\t\t\t\t\tdocument.getElementById(\"HD\").innerHTML=sensorValues[4]+\"%\";\n"
"\t\t\t\t\tdocument.getElementById(\"P\").innerHTML=sensorValues[5]+\"mBar\";\n"
"\t\t\t\t\tdocument.getElementById(\"PM10\").innerHTML=sensorValues[6]+\"&micro;g/m<sup>3</sup>\";\n"
"\t\t\t\t\tdocument.getElementById(\"PM25\").innerHTML=sensorValues[7]+\"&micro;g/m<sup>3</sup>\";\n"
"\t\t\t\t\tdocument.getElementById(\"RSSI\").innerHTML=sensorValues[11];\n"
"\t\t\t\t}\n"
"\t\t\t}\n"
"\t\t};\n"
"\txhttp.open(\"GET\", \"getSensors\", true);\n"
"\txhttp.send();}\n"
"</script>\n</head>\n";
