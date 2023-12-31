#include <stdio.h>

typedef struct {
    int row;
    int col;
    double value;
} COOElement;

int main() {
    FILE *inputFile = fopen("/media/zh/LENOVO_USB_HDD/for_jinzhou/3.mtx", "r");
    FILE *outputFile = fopen("/media/zh/LENOVO_USB_HDD/for_jinzhou/3_new.mtx", "w");

    if (inputFile == NULL || outputFile == NULL) {
        perror("Error opening file");
        return 1;
    }
    fprintf(outputFile, "%%%%MatrixMarket matrix coordinate pattern symmetric\n");
    // 读取矩阵的行列数和非零元素的数量
    long long rows, cols, nonZeros;
    fscanf(inputFile, "%lld %lld %lld", &rows, &cols, &nonZeros);

    // 输出矩阵的行列数和非零元素的数量
    fprintf(outputFile, "%lld %lld %lld\n", rows, cols, nonZeros);
    // 读取文件中的数据
    long long row, col;
    double real, imag;

    while (fscanf(inputFile, "%lld %lld (%le, %le)", &row, &col, &real, &imag) == 4) {
        double value = real; // 计算实部和虚部的和作为COO三元组的值
        fprintf(outputFile, "%lld %lld %.12e\n", row, col, value);
    }

    fclose(inputFile);
    fclose(outputFile);

    return 0;
}
