
from socket import *
def main():
    serverPort = 12000
    serverSocket = socket(AF_INET,SOCK_STREAM)
    serverSocket.bind(('127.0.0.2',serverPort))
    serverSocket.listen(1)
    print ('The server is ready to receive')
    while 1:
        connectionSocket, addr = serverSocket.accept()
        print(addr)
        print(connectionSocket, addr)
        sentence = connectionSocket.recv(1024)
        capitalizedSentence = sentence.upper()
        connectionSocket.send(capitalizedSentence)
        connectionSocket.close()

if __name__ == "__main__":
    main()
