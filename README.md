#Operator System Project
##A project emulate rsync

client syntax: source_address destination_address options

source_address & destination_address (pending)
- for local: folder_path
- for outer_computer: ip_address@folder_path

option
- get: one way from server to client
- put: one way from client to server
- post: two ways sync

Version_1 with GET option, get file from server to client (oneway)
user guide
```
git clone https://github.com/DucHuySET/os_project.git
cd os_project
cd server
gcc server.c ../utils/*.c ../include/*.c  -o server -lssl -lcrypto
./server
```
result: Server listening on port 12345...
```
cd ..
cd client
gcc client.c ../utils/*.c ../include/*.c  -o client -lssl -lcrypto
./client
```
result: 
Connected to server

Enter a command:

Input: "src dest GET"

Then observe result
