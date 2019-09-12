Built a bit torrent like file sharing system with fallback multi-tracker system with synchronization and parallel
downloading. Used openssl library for computing hash values of files. Implemented own algorithm for data piece selection in order to download efficiently a file from multiple servers(peers) piece by piece.

Every client resides on localhost (127.0.0.1)
Steps to run :

1. start tracker (one two or both)
2. give PORT NUMBER
3. start 'n' number of clients : Dont pass arguments while running the executable
4. place all the client executables in different directories
5. The runnable will ask for UPLOAD PORT ,TRACKER PORTs

Functions:

1.share
2.download
3.remove seeder
4.show downloads
5.to close application . press "quit".so as to notify the tracker to remove the current client from seeder list

