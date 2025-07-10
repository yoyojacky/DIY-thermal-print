#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <iconv.h>

void convertToGBK(const char* input, char* output) {
	iconv_t cd = iconv_open("GBK", "UTF-8");
	if (cd == (iconv_t)-1) {
		perror("iconv_open");
		exit(EXIT_FAILURE);
	}
	
	size_t in_len = strlen(input) + 1;
	size_t out_len = in_len * 2; 
	char* in_buf = (char*)input;
	char* out_buf = output;

	size_t in_left = in_len;
	size_t out_left = out_len;

	if (iconv(cd, &in_buf, &in_left, &out_buf, &out_left) == (size_t)-1) {
		perror("iconv");
		iconv_close(cd);
		exit(EXIT_FAILURE);
	}

	*out_buf = '\0';
	iconv_close(cd);
}


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


// main主函数

int main(int argc, char* argv[]) {
    const char* port_name = "/dev/ttyAMA0";
    int fd = open_serial_port(port_name);
    if (fd < 0) {
        return 1;
    }

    configure_serial_port(fd);

    // 初始化打印机
    unsigned char init_command[] = {0x1B, 0x40, 0x1D, 0x21,0x00};
    send_printer_command(fd, init_command, sizeof(init_command));

    char input[20480];
    if (argc > 1) {
	   strncpy(input, argv[1], sizeof(input) -1);
	   input[sizeof(input) - 1] = '\0';
    } else {
        printf("请输入字符串(UTF-8编码) :");
    	if (fgets(input, sizeof(input), stdin) == NULL) {
	    perror("fgets");
	    close(fd);
	    return EXIT_FAILURE;
    		}
     
    	size_t len = strlen(input);
    	if (len > 0 && input[len-1] == '\n') {
	    input[len - 1] = '\0';
    		}
    }

    char output[40960];
    convertToGBK(input, output);

    unsigned char command1[40960]; 
    size_t command1_len = 0;

    unsigned char prefix[] = { 0x1D, 0x21, 0x00};
    memcpy(command1, prefix, sizeof(prefix));
    command1_len += sizeof(prefix); 

    size_t output_len = strlen(output);
    memcpy(command1 + command1_len, output, output_len);
    command1_len += output_len; 

    send_printer_command(fd, command1, command1_len);


    // 再次初始化打印机
    send_printer_command(fd, init_command, sizeof(init_command));

    // 关闭串口
    close(fd);

    printf("\033[32m命令已发送到打印机!\033[0m 状态:\033[32mOK\033[0m\n");

    return 0;
}
