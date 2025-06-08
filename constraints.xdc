# FPGA秒表计时器约束文件
# 适用于Xilinx FPGA开发板

# 时钟约束
create_clock -period 20.000 -name sys_clk [get_ports clk]
set_property PACKAGE_PIN W5 [get_ports clk]
set_property IOSTANDARD LVCMOS33 [get_ports clk]

# 复位信号
set_property PACKAGE_PIN U18 [get_ports rst_n]
set_property IOSTANDARD LVCMOS33 [get_ports rst_n]

# 按键输入 (KEY0-KEY6)
set_property PACKAGE_PIN T18 [get_ports {key[0]}]
set_property PACKAGE_PIN W19 [get_ports {key[1]}]
set_property PACKAGE_PIN T17 [get_ports {key[2]}]
set_property PACKAGE_PIN U17 [get_ports {key[3]}]
set_property PACKAGE_PIN U19 [get_ports {key[4]}]
set_property PACKAGE_PIN V19 [get_ports {key[5]}]
set_property PACKAGE_PIN W18 [get_ports {key[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {key[*]}]

# 数码管段选信号 (a,b,c,d,e,f,g,dp)
set_property PACKAGE_PIN W7 [get_ports {seg[0]}]
set_property PACKAGE_PIN W6 [get_ports {seg[1]}]
set_property PACKAGE_PIN U8 [get_ports {seg[2]}]
set_property PACKAGE_PIN V8 [get_ports {seg[3]}]
set_property PACKAGE_PIN U5 [get_ports {seg[4]}]
set_property PACKAGE_PIN V5 [get_ports {seg[5]}]
set_property PACKAGE_PIN U7 [get_ports {seg[6]}]
set_property PACKAGE_PIN V7 [get_ports {seg[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {seg[*]}]

# 数码管位选信号 (6位数码管)
set_property PACKAGE_PIN U2 [get_ports {dig[0]}]
set_property PACKAGE_PIN U4 [get_ports {dig[1]}]
set_property PACKAGE_PIN V4 [get_ports {dig[2]}]
set_property PACKAGE_PIN W4 [get_ports {dig[3]}]
set_property PACKAGE_PIN V7 [get_ports {dig[4]}]
set_property PACKAGE_PIN W7 [get_ports {dig[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {dig[*]}]

# LED输出
set_property PACKAGE_PIN U16 [get_ports {led[0]}]
set_property PACKAGE_PIN E19 [get_ports {led[1]}]
set_property PACKAGE_PIN U19 [get_ports {led[2]}]
set_property PACKAGE_PIN V19 [get_ports {led[3]}]
set_property PACKAGE_PIN W18 [get_ports {led[4]}]
set_property PACKAGE_PIN U15 [get_ports {led[5]}]
set_property PACKAGE_PIN U14 [get_ports {led[6]}]
set_property PACKAGE_PIN V14 [get_ports {led[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led[*]}]

# 时序约束
set_input_delay -clock sys_clk -min 0.000 [get_ports {key[*]}]
set_input_delay -clock sys_clk -max 2.000 [get_ports {key[*]}]
set_output_delay -clock sys_clk -min 0.000 [get_ports {seg[*]}]
set_output_delay -clock sys_clk -max 2.000 [get_ports {seg[*]}]
set_output_delay -clock sys_clk -min 0.000 [get_ports {dig[*]}]
set_output_delay -clock sys_clk -max 2.000 [get_ports {dig[*]}]
set_output_delay -clock sys_clk -min 0.000 [get_ports {led[*]}]
set_output_delay -clock sys_clk -max 2.000 [get_ports {led[*]}]