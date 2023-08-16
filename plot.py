import matplotlib.pyplot as plt
import numpy as np

# 读取数据文件
data = np.genfromtxt('data.csv', delimiter=',')

# 提取时间和解的数值
t = data[:, 0]
x = data[:, 1]

# 绘制图形
plt.plot(t, x)
plt.xlabel('t')
plt.ylabel('x')
plt.title('Solution of Difference Equation')
plt.grid(True)
plt.show()