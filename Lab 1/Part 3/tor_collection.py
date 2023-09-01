from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from tbselenium.tbdriver import TorBrowserDriver
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

with TorBrowserDriver("/home/kali/tor-browser/", tbb_logfile_path="text.txt", executable_path="/home/kali/bin/geckodriver", headless=True) as driver:
    for site_i in range(0, 10):
        site = SITES[site_i]
        driver.get(site)
        path = "/home/kali/lab1q3/output/" + str(site_i)
        if not os.path.exists(path):
            os.mkdir(path)

        for i in range(0, 10):
            print("Capturing " + site + "\t Iteration " + str(i))
            p = subprocess.Popen(["sudo", "tcpdump", "--print", "-w", "/home/kali/lab1q3/output/" + str(site_i) + "/" + str(i) + ".pcap"]) 
            driver.refresh()
            time.sleep(20)
            p.terminate()  
            time.sleep(5)

driver.quit()
