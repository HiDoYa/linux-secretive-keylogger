# client_script.py
This script performs the necessary operations to encrypt and hide messages into a random image taken from memegenerator API. Afterwards the image is sent through twitter as a covert channel. 

This script is automated to perform the following operation every few minutes (can be changed according to usecase). 

In order not to bombard with tweets, the script perform some check. If the log file has enough text then it will proceed, if the log file is still empty or has very few characters then it will wait to send it again next time. 



# server_script.py

This is the script used for gathering the data from Twitter and decrypting into a readable file. 

# config (configuration file)

New addition for easier change in configuration of the script. This configuration file is used to change the API keys of the twitter bot, scheduling time, AES password, memeURL, etc. You can add a new configuration by the following format. 

[ key ] = [ value ] 

The script contains a function to parse the configuration file and store into a dictionary for easy usage. The parser separates key-value pairs with delimiter '=' so becareful when you have '=' sign on a value because this will remove it. 
