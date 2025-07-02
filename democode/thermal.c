#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>


// open serial port 
int open_serial_port(const char* port_name) {
	int fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		perror("打开串口失败");
		return -1;
	}
	return fd;
}

// 配置串口 
void configure_serial_port(int fd) {
	struct termios options;
	if (tcgetattr(fd, &options) < 0){
		perror("获取串口属性失败");
		return ;
	}

	// 设置波特率
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	options.c_cflag &= ~PARENB; // 无校验位
	options.c_cflag &= ~CSTOPB; // 1bit停止位
	options.c_cflag &= ~CSIZE; // 清除数据位掩码
	options.c_cflag |= CS8;    // 8bit 数据位
	options.c_cflag |= CREAD | CLOCAL;    //启用接收器，忽略控制信号 
	
	options.c_cc[VMIN] = 1;
	options.c_cc[VTIME] = 5; 

	if (tcsetattr(fd, TCSANOW, &options) < 0) {
		perror("设置串口属性失败");
	}
}


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
	
	unsigned char init_command[] = {0x1B, 0x40};
	send_printer_command(fd, init_command, sizeof(init_command));

	// 要测试的打印方式 
	/*
	unsigned char print_modes[] = {0x00, 0x01, 0x02, 0x04, 0x10, 0x20, 0x80};
	
	for (int i=0; i < sizeof(print_modes); i++) {
    	unsigned char set_print_mode_command[] = {0x1B, 0x21, print_modes[i]};  // ESC ! 01
    send_printer_command(fd, set_print_mode_command, sizeof(set_print_mode_command));

	char print_string[30];
	sprintf(print_string, "模式: 0x%02X 测试文本\n", print_modes[i]);
	send_printer_command(fd, (unsigned char*)print_string, strlen(print_string));

	unsigned char newline[] = {'\r', '\n'};
	send_printer_command(fd, newline, sizeof(newline));
	}
	*/

	char print_string[] = "Hello, Printer!"; // ASCII 字符串
	send_printer_command(fd, (unsigned char*)print_string, strlen(print_string));

	unsigned char newline[] = {'\r', '\n'};
	send_printer_command(fd, newline, sizeof(newline));

	unsigned char qrinit[] = {0x1b, 0x40};
    send_printer_command(fd, qrinit, sizeof(qrinit));

	unsigned char qrsize[] = {0x1d,0x28,0x6b,0x03,0x00,0x31,0x43,0x08};
    send_printer_command(fd, qrsize, sizeof(qrsize));

	unsigned char qr2[] = {0x1d,0x28,0x6b,0x03,0x00,0x31,0x45,0x30};
    send_printer_command(fd, qr2, sizeof(qr2));

	unsigned char qrcontent[] = {0x1d,0x28,0x6b,0x06,0x00,0x31,0x50,0x30,0x41,0x42,0x43};
    send_printer_command(fd, qrcontent, sizeof(qrcontent));

	unsigned char qrpos[] = {0x1b,0x61,0x01};
    send_printer_command(fd, qrpos, sizeof(qrpos));

	unsigned char qrguding[] = {0x1d,0x28,0x6b,0x03,0x00, 0x31, 0x52, 0x30};
    send_printer_command(fd, qrguding, sizeof(qrguding));

	unsigned char qrguding2[] = {0x1d,0x28,0x6b,0x03,0x00, 0x31, 0x51, 0x30};
    send_printer_command(fd, qrguding2, sizeof(qrguding2));

	unsigned char qrguding3[] = {0x1b,0x40,0x1d,0x21,0x00};
    send_printer_command(fd, qrguding3, sizeof(qrguding3));

	unsigned char qrguding4[] = {0x1b,0x61,0x01};
    send_printer_command(fd, qrguding4, sizeof(qrguding4));

	unsigned char qrguding5[] = {0xC9, 0xA8, 0xD2, 0xBB,0xC9,0xA8,0xB9, 0xD8, 0xD7,0xA2,0x0d,0x0a,0x0d,0x0a,0x0d,0x0a,0x0d,0x0a,0x0d,0x0a,0x1b,0x69};
    send_printer_command(fd, qrguding5, sizeof(qrguding5));

	/*
    unsigned char print_string_command[] = {'0', '1', '2', '3','4', '5', '\r', '\n'};
    send_printer_command(fd, print_string_command, sizeof(print_string_command));
	*/

	/*
	unsigned char test_page_command[] = {0x12, 0x54}; 
	send_printer_command(fd, test_page_command, sizeof(test_page_command));
	*/

	close(fd);
	return 0;
}

