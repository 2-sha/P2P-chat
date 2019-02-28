# Local P2P Chat
This is a fully decentralized chat. To communicate, simply run it on computers in a single local network (using one port). All messages are encoded in UTF-16, so national characters are supported, the only restriction is the font of the console.

You can use `-p` or `-port` argument to select the port you want to use (The default port is 8001)
``` bash
./chat.exe -port 1337
```

![Пример работы](https://image.prntscr.com/image/qJkkhUfPTYqk6M1j_pAuLQ.gif)

***
**And it's cross-platform!**<br>
It was tested on Windows 7, 10, Ubuntu and Gentoo, so I can't vouch for other Unix systems and older versions of Windows. But it still should work anywhere.
***
## Installation
The fastest and easiest way is to download the compiled files for Windows or Linux. Well, if someone wants to compile it on their own, there is CMake. It uses boost as a dependency, so make sure it's installed.
### Windows
[Link to compilled version](https://github.com/2-sha/P2P-chat/releases/download/v1.2/chat.exe)

#### Installing Boost
You can compile boost from [sources](https://www.boost.org/users/history/version_1_67_0.html) by yourself or install it by it's [installer](https://sourceforge.net/projects/boost/files/boost-binaries/1.67.0/). Cmake looks for Boost in the C:/local folder.
#### Compilation
Use visual studio ¯\_(ツ)_/¯ It works with CMake projects.

### Linux
[Link to compilled version](https://github.com/2-sha/P2P-chat/releases/download/v1.2/chat)

#### Installing Boost
If you wish, you can also compile from the [source code](https://www.boost.org/users/history/version_1_67_0.html), but you can just download the compiled version
``` bash
sudo apt-get install libboost-all-dev
```
#### Compilation
``` bash
git clone https://github.com/2-sha/P2P-chat.git
cd P2P-chat
cmake .
make
```
## How it works
It is based on UDP datagrams that are sent by a broadcast request to the network and are not encrypted in any way. If you wish, you can write your client that will work correctly with mine or just view messages using wireshark or something like it. The structure of all data sent is as follows (Yes, this is JSON):
```json
 {"type": "", "data": [{"":""}] }
```
For example, this is information about sending a message:
```json
 {"type": "message", 
    "data": [
        { "user": "2sha" },
        { "content": "Hello, everybody!" }
    ]}
```
There are 5 types of data:
  - `message` - the user sent a message
    - `user` - name of the user (wow)
    - `content` - the content of the message (so unexpectedly)
  - `add_user` - someone's joined to our chat
    - `user` - stranger's name
  - `remove_user` - someone left our chat
    - `user` - name of miserable
  - `user_list_req` - someone wants to know the list of users on the network. We are obliged to respond to this message
  - `user_list_res` - response to `user_list_req` request
    - `user` - our username

    
