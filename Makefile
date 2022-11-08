all:
	gcc main.c libiot_parser.so -o iotpst -Wall -Wextra -O3 -Wl,-rpath,'$$ORIGIN'