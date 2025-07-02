#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// 打开串口函数
int open_serial_port(const char* port_name) {
    int fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("打开串口失败");
        return -1;
    }
    return fd;
}

// 配置串口函数
void configure_serial_port(int fd) {
    struct termios options;
    if (tcgetattr(fd, &options) < 0) {
        perror("获取串口属性失败");
        return;
    }

    // 设置波特率（根据你的打印机要求设置）
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    // 设置数据位、停止位和校验位
    options.c_cflag &= ~PARENB;  // 无校验位
    options.c_cflag &= ~CSTOPB;  // 1 位停止位
    options.c_cflag &= ~CSIZE;   // 清除数据位掩码
    options.c_cflag |= CS8;      // 8 位数据位

    // 启用接收器，忽略控制信号
    options.c_cflag |= CREAD | CLOCAL;

    // 设置最小字符数和等待时间（非规范模式）
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 5;

    // 应用设置
    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("设置串口属性失败");
    }
}

// 发送打印机指令函数
void send_printer_command(int fd, const unsigned char* command, size_t length) {
    if (write(fd, command, length) != length) {
        perror("发送命令失败");
    }
}

int main() {
    const char* port_name = "/dev/ttyAMA0";
    int fd = open_serial_port(port_name);
    if (fd < 0) {
        return 1;
    }

    configure_serial_port(fd);

    // 初始化打印机
    unsigned char init_command[] = {0x1B, 0x40};
    send_printer_command(fd, init_command, sizeof(init_command));

    // 设置二维码大小为 43 03
    unsigned char qr_size_command[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x43, 0x08};
    send_printer_command(fd, qr_size_command, sizeof(qr_size_command));

    // 固定指令
    unsigned char fixed_command_1[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x45, 0x30};
    send_printer_command(fd, fixed_command_1, sizeof(fixed_command_1));

    // 设置二维码内容为 "https://www.52pi.com"
    // URL 转换为 ASCII 字节序列
    unsigned char qr_data[] = {
        0x1D, 0x28, 0x6B, 0x19, 0x00,
		0x31, 0x50, 0x30,
		0x68, 0x74, 0x74,
		0x70, 0x73, 0x3A, 
		0x2F, 0x2F, 0x77, 
		0x77, 0x77, 0x2E,
		0x35, 0x32, 0x70,
		0x69, 0x2E, 0x63, 0x6F,0x6D, 
        0x0d, 0x0a,
    };
    send_printer_command(fd, qr_data, sizeof(qr_data));

    // 二维码居中
    unsigned char qr_align_command[] = {0x1B, 0x61, 0x01};
    send_printer_command(fd, qr_align_command, sizeof(qr_align_command));

    // 固定指令
    unsigned char fixed_command_2[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x52, 0x30};
    send_printer_command(fd, fixed_command_2, sizeof(fixed_command_2));

    unsigned char fixed_command_3[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x51, 0x30};
    send_printer_command(fd, fixed_command_3, sizeof(fixed_command_3));

    // 打印文本 "扫一扫关注"
    unsigned char text_command[] = {
        0x1B, 0x40,
        0x1B, 0x61, 0x01,
        0x1D, 0x21, 0x00,
        0xC9, 0xA8, 0xD2, 0xBB, 0xC9, 0xA8, 0xB9, 0xD8, 0xD7, 0xA2,
		0x0d, 0x0a, 0x1b, 0x69,
        '\r', '\n'
    };
    send_printer_command(fd, text_command, sizeof(text_command));


    // 再次初始化打印机
    send_printer_command(fd, init_command, sizeof(init_command));

    // 关闭串口
    close(fd);

    printf("二维码和文本已发送到打印机。\n");

    return 0;
}
