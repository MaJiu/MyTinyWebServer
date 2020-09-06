import requests

url = input("输入url\n")
headers = {
        "Connection" : "keep-alive",
        "test" : "test"
        }
response = requests.get(url, headers=headers)
