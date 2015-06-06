#!/bin/sh

# generating a private key
openssl genrsa -des3 -out server.key 1024

# generating certificate signing request
openssl req -new -key server.key -out server.csr

# signing certificate with private key
openssl x509 -req -days 3650 -in server.csr -signkey server.key -out server.crt

# removing password requirement
cp server.key server.key.secure
openssl rsa -in server.key.secure -out server.key

# generating dhparam file
openssl dhparam -out dh512.pem 512
