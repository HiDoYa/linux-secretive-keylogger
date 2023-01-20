import os
from io import BytesIO
import tweepy
from stegano import lsb
import PIL
import requests

# reveal hidden message from image
def destenographize_image(image_path):
    message = lsb.reveal(image_path)
    # remove b'' strings from message
    message = message[2:]
    return message


def decrypt(password, encrypted_file, logged_file):
    enc_scheme = "-aes-128-cbc"
    opts = "-d -a -A -nosalt"
    openssl_cmd = (
        f"openssl enc -pass pass:{password} {enc_scheme} {opts} -in {encrypted_file} -out {logged_file} -nosalt 2> /dev/null "
    )

    try:
        os.system(openssl_cmd)
    except Exception as e:
        print(e)


# Setup dir structure
def setup_dir():
    if not os.path.exists("data"):
        os.mkdir("data")


# Delete temp files
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


def get_tweets(tw_api, image_path, data_path):
    username = config["USERNAME"]
    tweets = tw_api.user_timeline(username, count=10)
    new_tweets_id = []
    new_tweets_media = []
    for tweet in tweets:
        tweet_data = tweet._json
        # Use id as filename (so there are no duplicates)
        tw_id = tweet_data["id"]
        tw_media = tweet_data["entities"]["media"][0]["media_url"]

        if not os.path.exists(data_path.format(tw_id)):
            new_tweets_id.append(tw_id)
            new_tweets_media.append(tw_media)
    return (new_tweets_id, new_tweets_media)


def get_config():
    config = {}
    with open("config", "r") as config_file:
        for line in config_file:
            if line == "\n":
                continue
            line = line.replace("=", " ")
            pairs = line.split()
            key = pairs[0]
            value = pairs[1]
            config[key] = value
    return config


def main():
    setup_dir()

    image_path = "./data/{}.png"
    data_path = "./data/{}.txt"
    password = config["AES_PASSWORD"]

    tw_api = tweet_setup()
    new_tweets = get_tweets(tw_api, image_path, data_path)

    tweet_ids = new_tweets[0]
    tweet_medias = new_tweets[1]

    for idx in range(len(tweet_ids)):
        tweet_id = tweet_ids[idx]
        tweet_media = tweet_medias[idx]

        # Save image from url
        image = requests.get(tweet_media)
        image_path = image_path.format(tweet_id)
        PIL.Image.open(BytesIO(image.content)).save(image_path)

        # Data file paths
        logged_file = data_path.format(tweet_id)
        encrypted_file = data_path.format(str(tweet_id) + "_enc")

        # Save message to file
        try:
            encrypted_message = destenographize_image(image_path)
        except Exception:
            continue

        with open(encrypted_file, "w") as file_encrypted:
            file_encrypted.write(encrypted_message)

        # Decrypt
        decrypt(password, encrypted_file, logged_file)

        # Cleanup
        files = [encrypted_file, image_path]
        cleanup(files)


config = get_config()
main()
