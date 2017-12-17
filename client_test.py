# Echo client program
import socket
from time import sleep

HOST = '192.168.141.182'    # The remote host
PORT = 50021              # The same port as used by the server

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.send('1:10:20:30\0'.encode())
sleep(1)
s.send('1:14:20:1\0'.encode())
sleep(1)
s.send('1:16:30:0\0'.encode())
sleep(1)
s.send('1:17:20:2\0'.encode())
sleep(1)
s.send('5:19:10:5\0'.encode())
s.close()
