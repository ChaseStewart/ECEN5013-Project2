# Echo client program
import socket

HOST = '192.168.141.110'    # The remote host
PORT = 50021              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.send('Hello, world\0'.encode())
#data = s.recv(1024)#
s.close()
#print ('Received', repr(data))