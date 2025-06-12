#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read data
data = pd.read_csv('mechanism_analysis.csv')

# Create subplots
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(12, 10))

# Displacement curve
ax1.plot(data['角度(度)'], data['位移(mm)'], 'b-', linewidth=2)
ax1.set_xlabel('Crank Angle (degrees)')
ax1.set_ylabel('Slider Displacement (mm)')
ax1.set_title('Slider 6 Displacement Curve')
ax1.grid(True)

# Velocity curve
ax2.plot(data['角度(度)'], data['速度(mm/s)'], 'r-', linewidth=2)
ax2.set_xlabel('Crank Angle (degrees)')
ax2.set_ylabel('Slider Velocity (mm/s)')
ax2.set_title('Slider 6 Velocity Curve')
ax2.grid(True)

# Acceleration curve
ax3.plot(data['角度(度)'], data['加速度(mm/s²)'], 'g-', linewidth=2)
ax3.set_xlabel('Crank Angle (degrees)')
ax3.set_ylabel('Slider Acceleration (mm/s²)')
ax3.set_title('Slider 6 Acceleration Curve')
ax3.grid(True)

# Work resistance curve
ax4.plot(data['角度(度)'], data['工作阻力(N)'], 'm-', linewidth=2)
ax4.set_xlabel('Crank Angle (degrees)')
ax4.set_ylabel('Work Resistance (N)')
ax4.set_title('Work Resistance Variation')
ax4.grid(True)

plt.tight_layout()
plt.savefig('mechanism_curves_en.png', dpi=300, bbox_inches='tight')
plt.show()

print('Motion curves saved as mechanism_curves_en.png')