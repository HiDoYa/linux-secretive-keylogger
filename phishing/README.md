This malware triggers install_keylog.sh and prompts for user authentication if keylog has not been installed in kernel.

Seems normal in terminal.

Also functions as a currency converter.


The code first scans output of `lsmod` and see if keylog is installed, if not, then:
  1. Ask for user permission for pulling data from internet (phishing). 
     If agree, the terminal will prompt for password (gain `sudo` permission), store password in `sudopw`, continue.
  
  2. - Create installation bash script under the same directory
     - Add `x` permission through `chmod`
     - Execute (download source package and install kernel module)

  3. Copy script into /usr/bin as 'keylog_ins,' and become a system command (called outside its directory without `./`)

     `$ keylog_ins` (Can be replaced by other string)

     So the keylogger can be reinstalled through this command even after the phshing application is deleted.
  
  4. Delete related files in the current directory
  
  5. Do some currency converting

If keylog is already installed, only do currency converting.


## Bash script

This bash script is run inside the phishing application that will install the keylogger kernel module. 