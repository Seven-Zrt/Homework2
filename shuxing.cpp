#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

typedef struct {
    long long row;
    long long col;
    long double real;
    long double imag;
} ComplexEntry;

typedef struct {
    long long rows;
    long long cols;
    long long nnz;
    ComplexEntry* entries;
} ComplexSparseMatrix;

void readComplexMTXFile(const char* filePath, ComplexSparseMatrix* matrix) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        perror("无法打开文件");
        exit(EXIT_FAILURE);
    }

    // 尝试从文件第一行读取非零元素数量
    if (fscanf(file, "%lld %lld %lld", &matrix->rows, &matrix->cols, &matrix->nnz) != 3) {
        // 读取失败，报告错误并退出
        perror("读取文件头失败");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // 检查非零元素数量是否为负数
    if (matrix->nnz < 0) {
        // 非零元素数量为负数，尝试从文件的第一行读取正确的非零元素数量
        if (fseek(file, 0, SEEK_SET) != 0) {
            // 定位文件失败，报告错误并退出
            perror("定位文件失败");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // 尝试从文件的第一行读取非零元素数量
        if (fscanf(file, "%lld", &matrix->nnz) != 1) {
            // 读取失败，报告错误并退出
            perror("读取非零元素数量失败");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // 检查非零元素数量是否为负数
        if (matrix->nnz < 0) {
            // 非零元素数量依然为负数，报告错误并退出
            fprintf(stderr, "非零元素数量为负数\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    matrix->entries = (ComplexEntry*)malloc(matrix->nnz * sizeof(ComplexEntry));

    for (long long i = 0; i < matrix->nnz; ++i) {
        fscanf(file, "%lld %lld (%Lf, %Lf)", &matrix->entries[i].row, &matrix->entries[i].col,
               &matrix->entries[i].real, &matrix->entries[i].imag);
        matrix->entries[i].row--;
        matrix->entries[i].col--;
    }

    fclose(file);
}

void calculateNonZeroStatistics(const ComplexSparseMatrix* matrix, const char* outputFilePath) {
    FILE* outputFile = fopen(outputFilePath, "w");
    if (outputFile == NULL) {
        perror("无法打开输出文件");
        exit(EXIT_FAILURE);
    }

    long long nonZeroCount = matrix->nnz;

    fprintf(outputFile, "非零元素数量: %lld\n", nonZeroCount);

    // 统计每行非零元素数量的平均值和最大值
    long long* rowNonZeroCount = (long long*)calloc(matrix->rows, sizeof(long long));
    long long maxRowNonZeroCount = 0;
    long long maxRowIdx = 0;
    long long minRowIdx = 0;

    // 统计每列非零元素数量的平均值和最大值
    long long* colNonZeroCount = (long long*)calloc(matrix->cols, sizeof(long long));
    long long maxColNonZeroCount = 0;
    long long maxColIdx = 0;
    long long minColIdx = 0;

    // 遍历矩阵元素，统计每行和每列的非零元素数量
    for (long long i = 0; i < matrix->nnz; ++i) {
        long long row = matrix->entries[i].row;
        long long col = matrix->entries[i].col;

        rowNonZeroCount[row]++;
        colNonZeroCount[col]++;
    }

    // 计算每行非零元素数量的平均值和最大值
    double rowNonZeroAverage = 0;
    long long minRowNonZeroCount = LLONG_MAX;

    for (long long i = 0; i < matrix->rows; ++i) {
        rowNonZeroAverage += (double)rowNonZeroCount[i];

        if (rowNonZeroCount[i] > maxRowNonZeroCount) {
            maxRowNonZeroCount = rowNonZeroCount[i];
            maxRowIdx = i + 1;
        }

        if (rowNonZeroCount[i] < minRowNonZeroCount) {
            minRowNonZeroCount = rowNonZeroCount[i];
            minRowIdx = i + 1;
        }
    }

    rowNonZeroAverage /= matrix->rows;

    // 计算每列非零元素数量的平均值和最大值
    double colNonZeroAverage = 0;
    long long minColNonZeroCount = LLONG_MAX;

    for (long long j = 0; j < matrix->cols; ++j) {
        colNonZeroAverage += (double)colNonZeroCount[j];

        if (colNonZeroCount[j] > maxColNonZeroCount) {
            maxColNonZeroCount = colNonZeroCount[j];
            maxColIdx = j + 1;
        }

        if (colNonZeroCount[j] < minColNonZeroCount) {
            minColNonZeroCount = colNonZeroCount[j];
            minColIdx = j + 1;
        }
    }

    colNonZeroAverage /= matrix->cols;

    // 判断是否为对称矩阵
    int isSymmetric = 1;
    for (long long i = 0; i < matrix->nnz; ++i) {
        long long row = matrix->entries[i].row;
        long long col = matrix->entries[i].col;
        long double value = matrix->entries[i].real;

        // 寻找对应位置的元素
        for (long long j = 0; j < matrix->nnz; ++j) {
            if (matrix->entries[j].row == col && matrix->entries[j].col == row) {
                // 对称位置的元素是否相等
                if (matrix->entries[j].real != value) {
                    isSymmetric = 0;
                    break;
                }
            }
        }

        if (!isSymmetric) {
            break;
        }
    }

    // 输出判断结果
    if (isSymmetric) {
        fprintf(outputFile, "矩阵是对称的\n");
    } else {
        fprintf(outputFile, "矩阵不是对称的\n");
    }
    // 计算稠密度
    double density = (double)matrix->nnz / ((double)matrix->rows * matrix->cols);
    fprintf(outputFile, "每行非零元素平均数: %lf\n", rowNonZeroAverage);
    fprintf(outputFile, "每列非零元素平均数: %lf\n", colNonZeroAverage);
    fprintf(outputFile, "拥有最多非零元素的行号和数量: %lld, %lld\n", maxRowIdx, maxRowNonZeroCount);
    fprintf(outputFile, "拥有最多非零元素的列号和数量: %lld, %lld\n", maxColIdx, maxColNonZeroCount);
    fprintf(outputFile, "每行非零元素最小数量: %lld, %lld\n", minRowNonZeroCount, minRowIdx);
    fprintf(outputFile, "每列非零元素最小数量: %lld, %lld\n", minColNonZeroCount, minColIdx);
    fprintf(outputFile, "稠密度: %lf\n", density);

    fclose(outputFile);

    free(rowNonZeroCount);
    free(colNonZeroCount);
}

int main() {
    const char* filePath = "/media/zh/LENOVO_USB_HDD/for_jinzhou/2.mtx";
    const char* outputFilePath = "nonzero_statistics.txt";

    ComplexSparseMatrix complexMatrix;
    readComplexMTXFile(filePath, &complexMatrix);

    calculateNonZeroStatistics(&complexMatrix, outputFilePath);

    // 释放动态分配的内存
    free(complexMatrix.entries);

    return 0;
}

