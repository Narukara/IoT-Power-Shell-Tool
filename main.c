#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libiot_parser.h"

static const char* usage = "Usage: iotpst -p port [-o filename]\n";

static volatile bool stop = false;
static void int_handler(int foo) {
    (void)foo;
    stop = true;
}

int main(int argc, char* argv[]) {
    extern int opterr;
    opterr = 0;
    char *port = NULL, *filename = NULL;
    for (int opt; (opt = getopt(argc, argv, "p:o:")) != -1;) {
        extern char* optarg;
        switch (opt) {
            case 'p':
                port = optarg;
                break;
            case 'o':
                filename = optarg;
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
    if (filename == NULL) {
        filename = "./out";
    }

    signal(SIGINT, int_handler);
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
            fprintf(stderr, "Connection lost\n");
            exit(EXIT_FAILURE);
        }
        if (stop) {
            iot_uart_request_close();
            fprintf(stderr, "\nGive up\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("Output %s\n", (iot_get_power_on()) ? "ON" : "OFF");
    printf("Voltage = %.3fV\n", iot_get_set_voltage() / 1000.0);
    printf("Current = %.3f%s\n", iot_get_set_current() / 1000.0,
           iot_get_current_uint() ? "A" : "mA");

    printf("Collecting data... press Ctrl+C to stop\n");
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        iot_uart_request_close();
        fprintf(stderr, "Cannot open file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    while (!stop) {
        uint8_t result = iot_parse();
        if (result == 0x01) {
            for (int i = 0; i < 16; i++) {
                fprintf(file, "%f\t%f\t", iot_get_voltage(i),
                        iot_get_current(i));
            }
            fputc('\n', file);
        }
        if (result == 0xFF) {
            fclose(file);
            iot_uart_request_close();
            fprintf(stderr, "Connection lost\n");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    iot_uart_request_close();
    printf("\nDone\n");
    return 0;
}
