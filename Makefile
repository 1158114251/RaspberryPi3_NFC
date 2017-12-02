CC=gcc
CFGLASS=-Wall -g -c -O2

all:send_nfc_wifi  recv_nfc_wifi

send_nfc_wifi:send_nfc_wifi.o
	$(CC) $< -l nfc -o $@

recv_nfc_wifi:recv_nfc_wifi.o
	$(CC) $< -l nfc -o $@

%.o:%.c
	$(CC) $(CFGLASS) $< -o $@

.PHONY:
	clean

clean:
	$(RM) -rf *.o  send_nfc_wifi  recv_nfc_wifi

