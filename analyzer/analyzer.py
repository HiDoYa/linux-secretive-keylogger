import re

def look_for_email(text):
    # Source: https://stackoverflow.com/questions/42407785/regex-extract-email-from-strings
    regex = re.compile('([a-zA-Z0-9._-]+@([a-zA-Z0-9_-]+\.)+[a-zA-Z0-9_-]+)')
    matches = regex.findall(text)
    emails = [match[0] for match in matches]
    return emails

def look_for_cc_card_num(text):
    # Source: https://www.regular-expressions.info/creditcard.html
    regex = re.compile('\\b\d{13,16}\\b')
    matches = regex.findall(text)
    return matches

def look_for_address(text):
    # Source: https://stackoverflow.com/questions/11456670/regular-expression-for-address-field-validation
    regex = re.compile('\d{1,5}\s\w.\s(\\b\w*\\b\s){1,2}\w*\.')
    matches = regex.findall(text)
    return matches

def look_for_phone_number(text):
    # Source: https://stackoverflow.com/questions/16699007/regular-expression-to-match-standard-10-digit-phone-number
    regex = re.compile('^(\+\d{1,2}\s)?\(?\d{3}\)?[\s.-]\d{3}[\s.-]\d{4}$')
    matches = regex.findall(text)

    # Assume anything with consecutive digits is suspicious
    regex = re.compile('\\b\d{8,12}\\b')
    matches += regex.findall(text)
    return matches

def look_for_ssh_pass(text):
    capture_next_line = False
    captured_strs = []
    for line in text.split("\n"):
        if capture_next_line:
            captured_strs[-1].append(line)
            capture_next_line = False
        elif "ssh " in line:
            capture_next_line = True
            captured_strs.append([line])

    return captured_strs


def main():
    with open("log_file.txt", "r") as f:
        text = f.read()

    result = f"""
    Emails:
    {look_for_email(text)}

    Credit cards:
    {look_for_cc_card_num(text)}

    Address:
    {look_for_address(text)}

    Phone number:
    {look_for_phone_number(text)}

    SSH Pass:
    {look_for_ssh_pass(text)}
    """

    with open("log_file_analyzed.txt", "w") as f:
        f.write(result)

main()