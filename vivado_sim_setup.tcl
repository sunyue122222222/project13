# Vivado仿真设置脚本
# 用于自动配置和运行FPGA秒表计时器仿真

# 设置仿真参数
set_property -name {xsim.simulate.runtime} -value {50ms} -objects [get_filesets sim_1]
set_property -name {xsim.simulate.log_all_signals} -value {true} -objects [get_filesets sim_1]
set_property -name {xsim.simulate.wdb} -value {vivado_testbench_behav.wdb} -objects [get_filesets sim_1]

# 设置顶层测试模块
set_property top vivado_testbench [get_filesets sim_1]
set_property top_lib xil_defaultlib [get_filesets sim_1]

# 添加仿真源文件（如果还没有添加）
if {[get_files vivado_testbench.v] == ""} {
    add_files -fileset sim_1 vivado_testbench.v
}

# 更新编译顺序
update_compile_order -fileset sim_1

# 启动仿真
launch_simulation

# 添加所有信号到波形窗口
add_wave {{/vivado_testbench/uut}} 
add_wave {{/vivado_testbench/clk}}
add_wave {{/vivado_testbench/rst_n}}
add_wave {{/vivado_testbench/key}}
add_wave {{/vivado_testbench/seg}}
add_wave {{/vivado_testbench/dig}}
add_wave {{/vivado_testbench/led}}

# 添加内部信号监控
add_wave -group "Control Signals" {{/vivado_testbench/freq_mode}}
add_wave -group "Control Signals" {{/vivado_testbench/precision_mode}}
add_wave -group "Control Signals" {{/vivado_testbench/timer_mode}}
add_wave -group "Control Signals" {{/vivado_testbench/setting_mode}}

add_wave -group "Counters" {{/vivado_testbench/count_up}}
add_wave -group "Counters" {{/vivado_testbench/count_down}}
add_wave -group "Counters" {{/vivado_testbench/display_value}}
add_wave -group "Counters" {{/vivado_testbench/initial_value}}

add_wave -group "Clock Signals" {{/vivado_testbench/clk_1hz}}
add_wave -group "Clock Signals" {{/vivado_testbench/clk_2hz}}
add_wave -group "Clock Signals" {{/vivado_testbench/clk_4hz}}

add_wave -group "Key Signals" {{/vivado_testbench/key_pulse}}
add_wave -group "Key Signals" {{/vivado_testbench/key_stable}}

# 设置波形显示格式
set_property radix unsigned [get_waves /vivado_testbench/count_up]
set_property radix unsigned [get_waves /vivado_testbench/count_down]
set_property radix unsigned [get_waves /vivado_testbench/display_value]
set_property radix unsigned [get_waves /vivado_testbench/initial_value]
set_property radix binary [get_waves /vivado_testbench/key]
set_property radix binary [get_waves /vivado_testbench/seg]
set_property radix binary [get_waves /vivado_testbench/dig]
set_property radix binary [get_waves /vivado_testbench/led]

# 运行仿真
run all

# 缩放波形以适应窗口
zoom fit

puts "仿真设置完成！"
puts "波形窗口已配置，可以观察所有重要信号。"
puts "仿真将运行50ms，涵盖所有测试用例。"