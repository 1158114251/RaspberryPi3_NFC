#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif // HAVE_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
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
  uint8_t  abtRx[MAX_FRAME_LEN];
  int  szRx;
  uint8_t  abtTx[] = "Hello Mars!";

  nfc_context *context;
  nfc_init(&context);
#define MAX_DEVICE_COUNT 2
  nfc_connstring connstrings[MAX_DEVICE_COUNT];
  size_t szDeviceFound = nfc_list_devices(context, connstrings, MAX_DEVICE_COUNT);
  // Little hack to allow using nfc-dep-initiator & nfc-dep-target from
  // the same machine: if there is more than one readers opened
  // nfc-dep-target will open the second reader
  // (we hope they're always detected in the same order)
  if (szDeviceFound == 1) {
    pnd = nfc_open(context, connstrings[0]);
  } else if (szDeviceFound > 1) {
    pnd = nfc_open(context, connstrings[1]);
  } else {
    printf("No device found.\n");
    return EXIT_FAILURE;
  }

  if (argc > 1) {
    printf("Usage: %s\n", argv[0]);
    return EXIT_FAILURE;
  }

  nfc_target nt = {
    .nm = {
      .nmt = NMT_DEP,
      .nbr = NBR_UNDEFINED
    },
    .nti = {
      .ndi = {
        .abtNFCID3 = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xff, 0x00, 0x00 },
        .szGB = 4,
        .abtGB = { 0x12, 0x34, 0x56, 0x78 },
        .ndm = NDM_UNDEFINED,
        /* These bytes are not used by nfc_target_init: the chip will provide them automatically to the initiator */
        .btDID = 0x00,
        .btBS = 0x00,
        .btBR = 0x00,
        .btTO = 0x00,
        .btPP = 0x01,
      },
    },
  };

  if (!pnd) {
    printf("Unable to open NFC device.\n");
    return EXIT_FAILURE;
  }
  printf("NFC device: %s opened\n", nfc_device_get_name(pnd));

  signal(SIGINT, stop_dep_communication);

  printf("NFC device will now act as: ");
  //print_nfc_target(nt, false);

loop_wait:
  printf("Waiting for initiator request...\n");
  if ((szRx = nfc_target_init(pnd, &nt, abtRx, sizeof(abtRx), 0)) < 0) {
    nfc_perror(pnd, "nfc_target_init");
    goto error;
  }

  printf("Initiator request received. Waiting for data...\n");
  if ((szRx = nfc_target_receive_bytes(pnd, abtRx, sizeof(abtRx), 0)) < 0) {
    nfc_perror(pnd, "nfc_target_receive_bytes");
    goto error;
  }
  abtRx[(size_t) szRx] = '\0';
  printf("Received: %s\n", abtRx);

  printf("Sending: %s\n", abtTx);
  if (nfc_target_send_bytes(pnd, abtTx, sizeof(abtTx), 0) < 0) {
    nfc_perror(pnd, "nfc_target_send_bytes");
    goto error;
  }
  printf("Data sent.\n");
  usleep(1000);
  goto loop_wait;

error:
  nfc_close(pnd);
  nfc_exit(context);
  return EXIT_SUCCESS;
}

