# Echo server program
import socket

# defines
HOST = '192.168.141.221'    # The remote host
PORT = 50021 # The same port as used by the server

#code 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST,PORT))
s.listen(1)

print("Listening on host %s:%d" % (HOST, PORT))

(conn, addr) = s.accept()
print("Connected by %d" % (addr[1]))

while 1:
    data = conn.recv(1024)
    print("Received %s" % (data))
    break

s.close()
