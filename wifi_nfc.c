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
  uint8_t  abtTx[] = "P2P transmit Sample--SmartFire.cn";

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

  if (nfc_initiator_select_dep_target(pnd, NDM_PASSIVE, NBR_212, NULL, &nt, 1000) < 0) {
    nfc_perror(pnd, "nfc_initiator_select_dep_target");
    goto error;
  }
  //print_nfc_target(nt, false);

  printf("Sending: %s\n", abtTx);
  int res;
  if ((res = nfc_initiator_transceive_bytes(pnd, abtTx, sizeof(abtTx), abtRx, sizeof(abtRx), 0)) < 0) {
    nfc_perror(pnd, "nfc_initiator_transceive_bytes");
    goto error;
  }

  abtRx[res] = 0;
  printf("Received: %s\n", abtRx);

  if (nfc_initiator_deselect_target(pnd) < 0) {
    nfc_perror(pnd, "nfc_initiator_deselect_target");
    goto error;
  }

error:
  nfc_close(pnd);
  nfc_exit(context);
  return EXIT_SUCCESS;
}


