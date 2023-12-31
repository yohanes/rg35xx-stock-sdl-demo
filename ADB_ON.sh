#!/system/bin/sh

echo "Starting ADB..."  
echo 0 > /sys/monitor/usb_port/config/run

# host -> device
echo 1 > /sys/monitor/usb_port/config/idpin_debug
#ls -l /
#/usbmond.sh USB_A_OUT && /usbmond.sh USB_B_IN

usb_gandroid0_enable="/sys/class/android_usb/android0/enable"
usb_gandroid0_idVendor="/sys/class/android_usb/android0/idVendor"
usb_gandroid0_idProduct="/sys/class/android_usb/android0/idProduct"
usb_gandroid0_functions="/sys/class/android_usb/android0/functions"
usb_gandroid0_lun0="/sys/class/android_usb/android0/f_mass_storage/lun0/file"
usb_gandroid0_lun1="/sys/class/android_usb/android0/f_mass_storage/lun1/file"
usb_gandroid0_iManufacturer="/sys/class/android_usb/android0/iManufacturer"
usb_gandroid0_iProduct="/sys/class/android_usb/android0/iProduct"

echo "USB_A_OUT" > /sys/monitor/usb_port/config/usb_con_msg


echo "------USB_B_IN: device mode" > /dev/console
	
echo "USB_B_IN" > /sys/monitor/usb_port/config/usb_con_msg

echo "Actions" > $usb_gandroid0_iManufacturer
echo "ToyCloud" > $usb_gandroid0_iProduct
echo '0' > $usb_gandroid0_enable
echo "10d6" > $usb_gandroid0_idVendor
	
echo "0c02" > $usb_gandroid0_idProduct
echo "mass_storage,adb" > $usb_gandroid0_functions
echo '1' > $usb_gandroid0_enable
# Don't start adbd by default
echo "------adb start in B_IN" > /dev/console
/system/bin/start adbd

