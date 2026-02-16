both client.c server.c:
	clang client.c -lcurses -o client.out
	clang server.c -o server

run client server:
	osascript -e "tell application \"Terminal\" to do script \"cd '/Users/zakreashlaibah/Documents/Code Files/C/TCP-Server' && ./server\""
	osascript -e "tell application \"Terminal\" to do script \"cd '/Users/zakreashlaibah/Documents/Code Files/C/TCP-Server' && ./client.out\""
	osascript -e "tell application \"Terminal\" to do script \"cd '/Users/zakreashlaibah/Documents/Code Files/C/TCP-Server' && ./client.out\""
	osascript -e "tell application \"Terminal\" to do script \"cd '/Users/zakreashlaibah/Documents/Code Files/C/TCP-Server' && ./client.out\""
