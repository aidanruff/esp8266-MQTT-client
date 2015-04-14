<html><head><title>Test</title>
<link rel="stylesheet" type="text/css" href="style.css">
</head>
<body>
<div id="main">
<h1>The LED</h1>
<p>
LED state is %ledstate%. Temperature = %temperature%.
</p>
<form method="post" action="led.cgi">
<input type="password" name="web_pass" value="%web_pass%"> Password<br /> 
<input type="submit" name="led" value="1">
<input type="submit" name="led" value="0">
</form>
</div>
</body></html>

