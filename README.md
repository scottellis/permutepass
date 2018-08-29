# permutepass 

A small utility to automate some username guessing for Linux
systems assuming we can read /etc/shadow.
 
Given the password field from /etc/shadow for a user and a seed hint, 
try permuting the seed using "clever password tricks" and then encrypting
the result using the algorithm and salt from the shadow entry and see if
the resulting value matches the target.

    ~/permutepass$ make
    gcc -O2 -Wall permutepass.c -lmhash -o permutepass

    ~/permutepass$ ./permutepass
    Usage: ./permutepass -t <target> seed
      -t       Target hash from /etc/shadow. Program stops if encryption of permuted value matches.
      seed     The seed string to be modified and run through crypt.


An example

The target user is ubuntu

    $ sudo grep ubuntu /etc/shadow
    ubuntu:$6$zk1C1m83$S5eGzVtPQ9LiHptUUlPyhVa05v6auauI.K7j08NA6rqvopCHbQZ.iiPzrg.MaukebTOFWqQ/1cLPQq4w4arAF/:17772:0:99999:7:::

Escape the '$' in the password field with '\' so the shell doesn't try to interpret them.

    ~/permutepass$ ./permutepass -t "\$6\$zk1C1m83\$S5eGzVtPQ9LiHptUUlPyhVa05v6auauI.K7j08NA6rqvopCHbQZ.iiPzrg.MaukebTOFWqQ/1cLPQq4w4arAF/" password
    Success: pa$$w0Rd4
    $6$zk1C1m83$S5eGzVtPQ9LiHptUUlPyhVa05v6auauI.K7j08NA6rqvopCHbQZ.iiPzrg.MaukebTOFWqQ/1cLPQq4w4arAF/
