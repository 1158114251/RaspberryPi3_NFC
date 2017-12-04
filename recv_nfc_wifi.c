//#ifdef HAVE_CONFIG_H
#  include "config.h"
//#endif // HAVE_CONFIG_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <nfc/nfc.h>

#include "utils/nfc-utils.h"

#define MAX_FRAME_LEN 264

static nfc_device *pnd;

int _System(const char * cmd, char *pRetMsg, int msg_len)  
{  
    FILE * fp;  
//    char * p = NULL;  
    int res = -1;  
    if (cmd == NULL || pRetMsg == NULL || msg_len < 0)  
    {  
        printf("Param Error!\n");  
        return -1;  
    }  
    if ((fp = popen(cmd, "r") ) == NULL)  
    {  
        printf("Popen Error!\n");  
        return -2;  
    }  
    else  
    {  
        memset(pRetMsg, 0, msg_len);  
        //get lastest result  
        while(fgets(pRetMsg, msg_len, fp) != NULL)  
        {  
            printf("Msg:%s",pRetMsg); //print all info  
        }  
  
        if ( (res = pclose(fp)) == -1)  
        {  
            printf("close popenerror!\n");  
            return -3;  
        }  
        pRetMsg[strlen(pRetMsg)-1] = '\0';  
        return 0;  
    }  
}  



static void stop_dep_communication(int sig)
{
  (void) sig;
  if (pnd)
    nfc_abort_command(pnd);
  else
    exit(EXIT_FAILURE);
}

int
main(int argc, const char *argv[])
{
  nfc_target   nt;
  uint8_t  abtRx[MAX_FRAME_LEN];
  uint8_t  abtTx[] = "what is the wifi passwd ?";
  char tmp[100]={0};
  int res;
  char *cmd = "bash ping_network.sh";  
  char a8Result[128] = {0};  
  int ret = 0;  
  char * str_start_save ;
  char * str_end_save;
  char *str_start_ =(char *)abtRx;




  if (argc > 1) {
    printf("Usage: %s\n", argv[0]);
    return EXIT_FAILURE;
  }

  nfc_context *context;
  nfc_init(&context);

  pnd = nfc_open(context, NULL);
  if (!pnd) {
    printf("Unable to open NFC device.\n");
    return EXIT_FAILURE;
  }
  printf("NFC device: %s\n opened", nfc_device_get_name(pnd));

  signal(SIGINT, stop_dep_communication);

  if (nfc_initiator_init(pnd) < 0) {
    nfc_perror(pnd, "nfc_initiator_init");
    goto error;
  }

  if (nfc_initiator_select_dep_target(pnd, NDM_PASSIVE, NBR_212, NULL, &nt, 0) < 0) {
    nfc_perror(pnd, "nfc_initiator_select_dep_target");
    goto error;
  }
  //print_nfc_target(nt, false);

  if ((res = nfc_initiator_transceive_bytes(pnd, abtTx, sizeof(abtTx), abtRx, sizeof(abtRx), 0)) < 0) {
    nfc_perror(pnd, "nfc_initiator_transceive_bytes");
    goto error;
  }

  abtRx[res] = 0;
  printf("Received: %s\n", abtRx);
  while(*str_start_ ++!=':');
         str_start_save=str_start_+1;

  while(*str_start_!='\n')
  {
     str_start_++;
  }
   *str_start_ ='\0';
 printf("+++++%s\n",str_start_save);

  while(*str_start_++!=":");
  str_end_save=str_start_+1;

sprintf(tmp,"echo \"network={\" >>1.txt");
  system(tmp);

  sprintf(tmp,"echo \"ssid=\\\"%s\\\"\" >>1.txt","ssid");
  system(tmp);

  sprintf(tmp,"echo \"psk=\\\"%s\\\"\" >>1.txt","ccc");
  system(tmp);

 
  sprintf(tmp,"echo \"}\" >>1.txt");
  system(tmp);
  system("killall wpa_supplicant");
  system("cp /etc/wpa_supplicant/wpa_supplicant.conf.bak  /etc/wpa_supplicant/wpa_supplicant.conf");
  system("wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf");
  
  ret  = _System(cmd, a8Result, sizeof(a8Result));  
  printf("ret = %d \na8Result = %s\nlength = %d \n", ret, a8Result, strlen(a8Result));  

  
if (nfc_initiator_deselect_target(pnd) < 0) {
    nfc_perror(pnd, "nfc_initiator_deselect_target");
    goto error;
  }

error:
  nfc_close(pnd);
  nfc_exit(context);
  return EXIT_SUCCESS;
}


