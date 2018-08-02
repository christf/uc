uc will accept one line of text on stdin and  write that to a unix socket.
Afterwards uc will read and print the response from this unix socket until EOF.
Besides providing this functionality it is a goal of uc to be specifically
light-weight.

Examples
uc can be used like this:
echo get_clients | uc /var/run/l3roamd.sock
echo | uc /var/run/fastd.sock

uc is free software. Please feel free to contribute.
