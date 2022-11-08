#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libiot_parser.h"

static const char* usage = "Usage: iotpower.sh -p [PORT]\n";

int main(int argc, char* argv[]) {
    extern int opterr;
    opterr = 0;
    char* port = NULL;
    for (int opt; (opt = getopt(argc, argv, "p:")) != -1;) {
        extern char* optarg;
        switch (opt) {
            case 'p':
                port = optarg;
                break;
            default:
                fprintf(stderr, usage);
                exit(EXIT_FAILURE);
        }
    }

    if (port == NULL) {
        fprintf(stderr, usage);
        exit(EXIT_FAILURE);
    }

    if (!iot_uart_open_s(port, strlen(port))) {
        fprintf(stderr, "Failed to open serial port %s\n", port);
        exit(EXIT_FAILURE);
    }
    printf("Connected to %s\n", port);

    iot_uart_send_initial();
    printf("Waiting for initialization...\n");
    while (1) {
        uint8_t result = iot_parse();
        if (result == 0x04) {
            break;
        }
        if (result == 0xFF) {
            iot_uart_request_close();
            fprintf(stderr, "Connection lost");
            exit(EXIT_FAILURE);
        }
    }

    printf("Output %s\n", (iot_get_power_on()) ? "ON" : "OFF");
    printf("Voltage = %.3fV\n", iot_get_set_voltage() / 1000.0);
    printf("Current = %.3f%s\n", iot_get_set_current() / 1000.0,
           iot_get_current_uint() ? "A" : "mA");

    iot_uart_request_close();

    return 0;
}