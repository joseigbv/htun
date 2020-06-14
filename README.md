# htun

Simple pentesting tool to establish reverse tcp tunnel through compromised http server. Useful in pentesting when you can execute commands on a remote server through the web and it is protected by a firewall with only the outgoing http port open.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

The client should run on any UNIX/Linux box. You only need a relatively modern gcc compiler. To compile for another architecture (eg Solaris SPARC) you will need a cross compiler. 

### Installing

Download a copy of the project from github: 

```
git clone https://github.com/joseigbv/htun
```

Edit 'config.h' and compile: 

```
$ make all
```

### Usage 

* 1. Edit, configure and upload the web tool (server.jsp or server.asp) to compromised server.
* 2. Upload the binary server (use your preferred webshell).
* 3. Execute the client in your pentest box: 

```
$ client -u http://compromised-hostname/server.jsp -b 127.00.1:2222 -f 0.0.0.0:22 
```

## Authors

* **José Ignacio Bravo** - *Initial work* - nacho.bravo@gmail.com

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
