from selenium import webdriver
from selenium.webdriver.chrome.options import Options
import sys
import os
import time
import subprocess

def capture_tcpdump(filepath=str, timeout=int):
    # start tcpdump capture
    print("Starting tcpdump capture...")

SITES = [
    "https://en.wikipedia.org/wiki/Cat",
    "https://en.wikipedia.org/wiki/Dog",
    "https://en.wikipedia.org/wiki/Egress_filtering",
    "http://web.mit.edu/",
    "http://www.unm.edu/",
    "https://www.cmu.edu/",
    "https://www.berkeley.edu/",
    "https://www.utexas.edu/",
    "https://www.asu.edu/",
    "https://www.utdallas.edu/"
]

driver = webdriver.Firefox()

for site_i in range(0, 10):
    site = SITES[site_i]
    driver.get(site)
    os.mkdir("output/" + str(site_i))

    for i in range(0, 10):
        print("Capturing " + site + "\t Iteration " + str(i))
        p = subprocess.Popen(["tcpdump", "-w", "output/" + str(site_i) + "/" + str(i) + ".pcap"]) 
        p.communicate()
        driver.refresh()
        time.sleep(5)
        p.send_signal(subprocess.signal.SIGTERM)
        time.sleep(5)


driver.get("http://www.python.org")
driver.quit()