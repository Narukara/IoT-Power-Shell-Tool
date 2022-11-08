#ifndef LIBIOT_PARSER_H
#define LIBIOT_PARSER_H

#include <stdint.h>

/**
 * @brief 打开串口
 * @note 仅支持windows平台。返回1表示串口打开成功
 */
uint8_t iot_uart_open(int32_t com);

/**
 * @brief 打开串口（字符串）
 * @note 传入com口的完整字符串名，如COM1、/dev/ttyS2。返回1表示串口打开成功
 */
uint8_t iot_uart_open_s(char* s, uint64_t len);

/**
 * @brief 关闭串口
 */
void iot_uart_request_close();

/**
 * @brief 发送初始化命令
 */
void iot_uart_send_initial();

/**
 * @brief 循环接收数据
 * @note
 * 该接口应单独开一个线程，并放到while(true)中循环处理。并且循环内无需延时处理，接口会自行阻塞防止吃满cpu
 * @return
 * 0x00     无数据，建议跳过本次循环(continue)
 * 0x01     收到电压电流数据，一共16包
 * 0x04     设备当前状态，发送初始化命令后才会回复
 * 0x06     成功设置设备状态回复
 * 0xff     串口断开，建议退出循环，并尝试重新连接
 * 其他值    这是收到了数据包，结合下面例子使用
 */
uint8_t iot_parse();

/**
 * @brief 读取数据（0x01）
 */
double iot_get_current(int32_t index);

/**
 * @brief 读取数据（0x01）
 */
double iot_get_voltage(int32_t index);

/**
 * @brief 读取设备当前状态（0x04）
 * @note
 * 只有在调用iot_uart_send_initial函数、并收到0x04包回复后，该数据才能保证为最新数据
 * @return 设备输出电流值的单位：1为A、0为mA
 */
uint8_t iot_get_current_uint();

/**
 * @brief 读取设备当前状态（0x04）
 * @note
 * 只有在调用iot_uart_send_initial函数、并收到0x04包回复后，该数据才能保证为最新数据
 * @return 设备当前设置输出的电流：为实际值*1000（0000-9999）
 */
uint16_t iot_get_set_current();

/**
 * @brief 读取设备当前状态（0x04）
 * @note
 * 只有在调用iot_uart_send_initial函数、并收到0x04包回复后，该数据才能保证为最新数据
 * @return 设备当前设置输出的电压：为实际值*1000（0000-9999）
 */
uint16_t iot_get_set_voltage();

/**
 * @brief 读取设备当前状态（0x04）
 * @note
 * 只有在调用iot_uart_send_initial函数、并收到0x04包回复后，该数据才能保证为最新数据
 * @return 设备当前输出状态：1开启输出、0关闭输出
 */
uint8_t iot_get_power_on();

/**
 * @brief 设置设备当前输出状态
 * @param current 设备当前设置输出的电流：为实际值*1000（0000-2000）
 * @param unit 设备输出电流值的单位：1为A、0为mA
 * @param voltage 设备当前设置输出的电压：为实际值*1000（0000-2000）
 * @param on 设备当前输出状态：1开启输出、0关闭输出
 * @note 当设置后，如果收到0x06数据包，则表示设置成功
 */
void iot_uart_send_set(int16_t current,
                       uint8_t unit,
                       uint16_t voltage,
                       uint8_t on);

/********************************
完整流程

//第一步打开串口
if(iot_uart_open(1)==1)//打开com1
{
    //必须发送一包初始化请求，并保证收到回复结果；如果没收到回复要重发
    iot_uart_send_initial();
    //必须保证收到了iot_parse()返回的0x04回复
    iot_parse().......//处理代码
    //收到其他值无所谓，只需要等0x04包就好，除非收到了0xff串口断了
    //如果等了几秒还没收到，考虑重发

    //确认收到初始化回复后，这里应该单独开一个线程收数据
    循环 {
        获取 iot_parse() 的返回值，并处理
        0xff: 串口断开，退出循环
        0x00: 没数据，忽略这一包，接收下一包数据
        0x01: 收到电压电流数据，一共16包，自行处理 {
            iot_get_current(0) 到 iot_get_current(15)
            iot_get_voltage(0) 到 iot_get_voltage(15)
        }
    }
    //关闭串口
    iot_uart_request_close();
}

********************************/

#endif