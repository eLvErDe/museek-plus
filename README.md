# Museek Plus

## No longer maintained!

Museek+ is no longer in active development. Please use one of the actively maintained alternatives to avoid compatibility issues with other Soulseek clients:

- [slskd](https://github.com/slskd/slskd), a modern client-server application
- [Nicotine+](https://nicotine-plus.org/), a graphical client for desktop
- [Seeker](https://github.com/jackBonadies/SeekerAndroid), a client for Android

## Setup

Website for ticket, documentation and packages is http://www.museek-plus.org

**Museek** consists of the binary progams: 
- museekd
- museeq
- muscan ( and possibly muscand).

The configuration script is:
- musetup

**Steps:**

1) Configure your settings with musetup .
 	- Settings that must be set for museekd to run are:
    Server, Username, Password, Interface Password, Download Directory.

 	- Having a shares directories is advised.

2) Run museekd. (The Museek Daemon)

3) Run a client such as museeq or mucous ( http://thegraveyard.org/daelstorm/mucous.php )

4) Login into museekd via the interface socket or host:port and the interface password.
