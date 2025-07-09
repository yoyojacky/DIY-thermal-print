#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

// 将输入字符串转换为GBK编码
void convertToGBK(const char* input, char* output) {
    iconv_t cd = iconv_open("GBK", "UTF-8");
    if (cd == (iconv_t)-1) {
        perror("iconv_open");
        exit(EXIT_FAILURE);
    }

    size_t in_len = strlen(input) + 1; // 包括字符串结束符
    size_t out_len = in_len * 2; // 最坏情况下，GBK编码长度可能是UTF-8的两倍
    char* in_buf = (char*)input;
    char* out_buf = output;

    size_t in_left = in_len;
    size_t out_left = out_len;

    if (iconv(cd, &in_buf, &in_left, &out_buf, &out_left) == (size_t)-1) {
        perror("iconv");
        iconv_close(cd);
        exit(EXIT_FAILURE);
    }

    *out_buf = '\0'; // 确保输出字符串以空字符结尾
    iconv_close(cd);
}

// 生成C语言变量初始化代码
void generateCVariable(const char* str, const char* varName) {
    printf("unsigned char %s[] = {", varName);
    while (*str) {
        printf("0x%02X, ", (unsigned char)*str);
        str++;
    }
    printf("};\n");
}

int main() {
    char input[1024];
    printf("请输入字符串（UTF-8编码）：");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        perror("fgets");
        return EXIT_FAILURE;
    }

    // 去掉换行符
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    char output[2048]; // 输出缓冲区
    convertToGBK(input, output);

    // 生成C语言变量初始化代码
    generateCVariable(output, "command1");

    return 0;
}
