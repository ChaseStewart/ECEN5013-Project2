#!/usr/bin/python
import socket

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('192.168.141.110', 50021))
#client_socket.connect(('localhost', 50010))

while True:
	in_str = client_socket.recv(1024)
	if (len(in_str)) > 5:
		print("Received %s!" % (in_str))
		client_socket.send("Ahoy matey!\x00")
		print("Sent \"Ahoy matey!\"")

