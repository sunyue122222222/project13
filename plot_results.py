#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

# 读取数据
data = pd.read_csv('mechanism_analysis.csv')

# 创建子图
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))

# 位移曲线
ax1.plot(data['角度(度)'], data['位移(mm)'], 'b-', linewidth=2)
ax1.set_xlabel('曲柄转角 (度)')
ax1.set_ylabel('滑块位移 (mm)')
ax1.set_title('滑块6位移曲线')
ax1.grid(True)

# 速度曲线
ax2.plot(data['角度(度)'], data['速度(mm/s)'], 'r-', linewidth=2)
ax2.set_xlabel('曲柄转角 (度)')
ax2.set_ylabel('滑块速度 (mm/s)')
ax2.set_title('滑块6速度曲线')
ax2.grid(True)

# 加速度曲线
ax3.plot(data['角度(度)'], data['加速度(mm/s²)'], 'g-', linewidth=2)
ax3.set_xlabel('曲柄转角 (度)')
ax3.set_ylabel('滑块加速度 (mm/s²)')
ax3.set_title('滑块6加速度曲线')
ax3.grid(True)

# 工作阻力曲线
ax4.plot(data['角度(度)'], data['工作阻力(N)'], 'm-', linewidth=2)
ax4.set_xlabel('曲柄转角 (度)')
ax4.set_ylabel('工作阻力 (N)')
ax4.set_title('工作阻力变化曲线')
ax4.grid(True)

plt.tight_layout()
plt.savefig('mechanism_curves.png', dpi=300, bbox_inches='tight')
plt.show()

print('运动曲线图已保存为 mechanism_curves.png')
