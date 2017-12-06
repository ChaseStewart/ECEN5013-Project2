#!/usr/bin/python
import socket

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('localhost', 50010))
client_socket.send('test yooo\0')
client_socket.close()
