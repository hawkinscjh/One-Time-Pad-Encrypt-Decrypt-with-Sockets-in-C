# One-Time-Pad-Encrypt-Decrypt-with-Sockets-in-C
Encrypt plaintext and decrypt ciphertext using a key. Encryption/decryption occurs on respective encryption and decryption servers.

To compile: run 'comipleall' from folder containing all relevant .c files.

![top](https://user-images.githubusercontent.com/59400213/227791079-b5890723-31b7-449a-b450-2023dc65af6d.jpg)


Example Usage:

$ cat plaintext1

THE RED GOOSE FLIES AT MIDNIGHT STOP

$ enc_server 57171 &

$ dec_server 57172 &

$ keygen 10

EONHQCKQ I

$ keygen 10 > mykey

$ cat mykey

VAONWOYVXP

$ keygen 10 > myshortkey

$ enc_client plaintext1 myshortkey 57171 > ciphertext1

Error: key ‘myshortkey’ is too short

$ echo $?

1

$ keygen 1024 > mykey

$ enc_client plaintext1 mykey 57171 > ciphertext1

$ cat ciphertext1

WANAWTRLFTH RAAQGZSOHCTYS JDBEGYZQDQ

$ keygen 1024 > mykey2

$ dec_client ciphertext1 mykey 57172 > plaintext1_a

$ dec_client ciphertext1 mykey2 57172 > plaintext1_b

$ cat plaintext1_a

THE RED GOOSE FLIES AT MIDNIGHT STOP

$ cat plaintext1_b

WSXFHCJAEISWQRNO L ZAGDIAUAL IGGTKBW

$ cmp plaintext1 plaintext1_a

$ echo $?

0

$ cmp plaintext1 plaintext1_b

plaintext1 plaintext1_b differ: byte 1, line 1

$ echo $?

1

$ enc_client plaintext5 mykey 57171

enc_client error: input contains bad characters

$ echo $?

1

$ enc_client plaintext3 mykey 57172

Error: could not contact enc_server on port 57172

$ echo $?

2

$

