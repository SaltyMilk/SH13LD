# SH13LD
Educational trojan horse.

:warning: PLEASE DON'T BE STUPID: DO NOT RUN THIS ON YOUR SYSTEM. :warning:
# How does it work
Make will generate an exec called Durex.

If you launch it, it'll simply print a string. 

**In the background here's what's happening**:
* Compiling and creating a malware daemon called /bin/Durex
* Config init.d to launch the malware at each boot of the system (call the malware service "antivirus")
* Execute the malware

**Here's what the malware does**:
* Log all keyboard inputs
* Backdoor (root shell on network), connecting to 4242 with a password will let you pop a root shell available on port 4243 
* The malware will trick programs such as netstat, top, htop making them believe it's called FirewallService and not /bin/Durex

# Run in docker
```
./run.sh
docker exec -it durex /bin/bash 
cd /durex/srcs; make
```
# Run locally
```
cd srcs; make
```
