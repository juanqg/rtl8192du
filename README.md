rtl8192du
=========

Source code for RTL8192DU device

Install Instructions:
# For ubuntu
    sudo apt-get install mokutil && mokutil --sb-state
    sudo apt-get install git linux-headers-generic build-essential dkms
    
# Get the source and build the driver
    git clone https://github.com/lwfinger/rtl8192du.git
    cd rtl8192du
    sudo dkms add .
    sudo dkms install 8192du/1.0
    Check installation with:
    lsmod | grep 8192du
    
    If driver is not listed, you may aswell:
    sudo modprobe 8192du
    
