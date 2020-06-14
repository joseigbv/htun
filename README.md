# htun

Simple pentesting tool to establish reverse tcp tunnel through compromised http server.

Useful in pentesting when you can execute commands on a remote server through the web and it is protected by a firewall with only the outgoing http port open.

Usage: 

* 1.- Edit, configure and upload the web tool (server.jsp or server.asp) to compromised server.
* 2.- Compile and upload the binary server (use your preferred webshell).
* 3.- Compile and execute the client in your pentest box: 

```
$ client -u http://compromised-hostname/server.jsp -b 127.00.1:2222 -f 0.0.0.0:22 
```
