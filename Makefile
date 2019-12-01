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
<<<<<<< HEAD
	qemu-system-x86_64 -L . -m 64 -fda Disk.img -hda HDD.img -boot a -localtime -M pc -rtc base=localtime
=======
	qemu-system-x86_64 -L . -fda Disk.img -hda HDD.img -boot a -m 64 -localtime -M pc -rtc base=localtime
>>>>>>> NEC
	
	brctl addbr br0	
	ip addr flush dev eth0
	brctl addif br0 eth0
	tunctl -t tap0 -u $(showami)
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
