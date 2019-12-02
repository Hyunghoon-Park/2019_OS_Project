all: BootLoader Kernel32 Kernel64 Utility Disk.img

BootLoader:
	@echo 
	@echo ============== Build Boot Loader ===============
	@echo 
	
	make -C 00.BootLoader

	@echo 
	@echo =============== Build Complete ===============
	@echo 

Kernel32:
	@echo
	@echo =============== Build 32bit Kernel ==============
	@echo

	make -C 01.Kernel32

	@echo
	@echo =============== Build Complete ===============
	@echo

Kernel64:
	@echo
	@echo =============== Build 32bit Kernel ==============
	@echo

	make -C 02.Kernel64

	@echo
	@echo =============== Build Complete ===============
	@echo

Utility:
	@echo
	@echo =============== Build 32bit Kernel ==============
	@echo

	make -C 04.Utility

	@echo
	@echo =============== Build Complete ===============
	@echo
	
Disk.img: 00.BootLoader/BootLoader1.bin 00.BootLoader/BootLoader2.bin 01.Kernel32/Kernel32.bin 02.Kernel64/Kernel64.bin
	@echo 
	@echo =========== Disk Image Build Start ===========
	@echo 

	./ImageMaker.exe $^

	@echo 
	@echo ============= All Build Complete =============
	@echo 

run:
	qemu-system-x86_64 -L . -fda Disk.img -hda HDD.img -boot a -netdev tap,id=network0,ifname=tap0,script=no,downscript=no -device e1000,netdev=network0,mac=52:53:54:55:56:56 -m 64 -localtime -M pc -rtc base=localtime -soundhw pcspk

	brclt addbr br0
	ipaddr flush dev eth0
	brctl addif br0 eth0
	tunctl -t tap0 -u $(whoami)
	brctl addif br0 tap0
	ifconfig eth0 up
	ifconfig tap0 up
	ifconfig br0 up


clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	make -C 02.Kernel64 clean
	make -C 04.Utility clean
	rm -f Disk.img
	rm -f ImageMaker.exe
