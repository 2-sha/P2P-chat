# Local P2P Chat
Chat is fully decentralized and to communicate simply run it on a computer in one local network (well, connect to one port). All messages are encoded in UTF-16, so national characters are supported, restrictions only the font of the console.

At startup, you can use `-p` or `-port` option to select the port to which you want to connect (The default port is 8001)
``` bash
./chat.exe -port 1337
```

![Пример работы](https://image.prntscr.com/image/qJkkhUfPTYqk6M1j_pAuLQ.gif)

***
**AND IT'S CROSS-PLATFORM!!!!**<br>
In general, I tested it only on Windows and Ubuntu, so for other Unix systems and older versions of Windows I can't vouch. But it should still work everywhere.
***
## Installation
The fastest and easiest way is to download the compiled files for Windows or Linux. Well, if someone wants to compile on their own, there is CMake. When creating used boost, so it will have to install.
### Windows
[Link to compilled version](https://github.com/2-sha/P2P-chat/releases/download/v1.0/P2P-chat_windows.exe)

##### Boost installing
You can compile boost from [sources](https://www.boost.org/users/history/version_1_67_0.html) yourself or install it by this [installer](https://sourceforge.net/projects/boost/files/boost-binaries/1.67.0/). Cmake looks for it in the folder C:/local
##### Compilation
Use visual studio ¯\_(ツ)_/¯ It can works with CMake projects

### Linux
[Link to compilled version](https://github.com/2-sha/P2P-chat/releases/download/v1.0/P2P-chat_linux)

##### Boost installing
If you wish, you can also compile from the [source code](https://www.boost.org/users/history/version_1_67_0.html), but you can just download the compiled version
``` bash
sudo apt-get install libboost-all-dev
```
##### Compilation
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

    
