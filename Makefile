wifi_nfc : wifi_nfc.c
	gcc wifi_nfc.c -o wifi_nfc -l nfc
clean: 
	rm -rf wifi_nfc
