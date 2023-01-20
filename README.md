# Kernel Level Keylogger with Twitter as Covert Channel

## Main ideas & Code Structure 

### Kernel Keylogger

Within the Source directory, we have our keylog.c file which is the keylogger which is essentially installed to the kernel as a kernel module. This keylogger works by intercepting the keyboard's event handler and adds a callback function so that every time a key is pressed. The custom function we wrote will be invoked. The function will store the key presses and other information (such as timing of keypresses) into a struct. This struct is the buffer in the memory to hold the keylogging data. We then eventually will store this data into a text file for processing. 

### Phishing Application 

Then, we have several directories for the other tools surrounding the keylogger. The PhishingApplication directory contains the source code and binary for the CurrencyConverter which serves to fool the user and install the keylogger into the system. This is all the victim needs to have the keylogger installed into their system. Once a user runs the phishing application. The application will download the keylogger program in the background, install the keylogger and run the scheduler. 


### Text analysis

The Analysis directory contains the tool to filter out useless keystrokes and obtain useful information from the keylogged files. Such as credit card info and user information. 

### Twitter bot and scheduler

The source code can be found in Scripts folder. The client_script performs scheduling, encrypting messages, stenographize image and sending it through Twitter. Contains server_script for the attacker to receive back the information. Contains configuration file for changing some other important values (GET URL, API keys, schedule time). The idea so that we can apply different twitter users too for different infected computers. 

## Keylogger and Covert Channel Explanation

The keylogger is installed in the kernel level, which makes it harder to detect. Program have been tested with antivirus software and remained undetected. 

To communicate with the attacker secretly, we use a covert channel through Twitter. We aren't directly communicating with the attacker so communication activity isn't as suspicious. Because Twitter is public actually anyone can retrieve back the information from any twitter profile, which makes it difficult to find who is actually looking at the photos. But the information is encrypted and discreet, it would be difficult for anyone to see if there is such information stored in the photo. So we can say we have a two layer protection in regards of keeping the information secret to the attacker. First the information is hidden and cannot be distinguished by the human eye. Second the information is encrypted and would be computatinally expensive to decrypt. It might also be difficult to tell if there is actually information because the encrypted text could just look like a series of random characters that is obtained from looking at the image pixels in a certain way. So they can never be certain that there is information inside. 

## Instructions on running the keylogger manually

Note: Script/api variables need to be updated to point to your twitter keys.

With the new update no hardcoded memory is needed

Run makefile.

`$ make`

Then, you'll get keylog.ko file. Insert the module into the kernel.

`$ sudo insmod keylog.ko`

You can double check to see if the kernel was loaded properly by checking if there is "keylog" inside:

`$ lsmod`

Keylogger module can be removed with:

`$ sudo rmmod keylog`

To see the logged file:

`$ sudo cat /proc/buffer_file`

The logged file will then be encrypted and decrypted with:

`$ openssl enc -pass pass:password -aes-128-cbc -a -A -in out.txt -out encrypted.txt -nosalt`

`$ openssl enc -d -pass pass:password -aes-128-cbc -a -A -in encrypted.txt -out new.txt -nosalt`

Tweets are published to: [Link](https://twitter.com/Lemon12776532)

## Credits
This project was initially created by Hiroya Gojo (myself), Marco Brian, Nan Chen, and Shuyao Li. I continued to work on this afterwards.

### Some Resources: 
- http://redgetan.cc/lets-write-a-kernel-keylogger/
- https://github.com/jarun/keysniffer/
- https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
- http://www.quadibloc.com/comp/scan.htm
