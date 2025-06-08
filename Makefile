# FPGA秒表计时器项目Makefile

# 工具链设置
IVERILOG = iverilog
VVP = vvp
GTKWAVE = gtkwave

# 源文件
SOURCES = stopwatch_top.v clock_divider.v key_debounce.v seg_display.v led_controller.v
TESTBENCH = testbench.v
TOP_MODULE = stopwatch_top
TB_MODULE = testbench

# 输出文件
VVP_FILE = $(TB_MODULE).vvp
VCD_FILE = $(TB_MODULE).vcd

# 默认目标
all: simulate

# 编译
compile: $(VVP_FILE)

$(VVP_FILE): $(SOURCES) $(TESTBENCH)
	$(IVERILOG) -o $(VVP_FILE) -s $(TB_MODULE) $(TESTBENCH) $(SOURCES)

# 仿真
simulate: $(VVP_FILE)
	$(VVP) $(VVP_FILE)

# 查看波形
wave: $(VCD_FILE)
	$(GTKWAVE) $(VCD_FILE) &

# 语法检查
syntax: $(SOURCES)
	$(IVERILOG) -t null $(SOURCES)

# 清理
clean:
	rm -f $(VVP_FILE) $(VCD_FILE) *.vcd

# 帮助
help:
	@echo "可用目标:"
	@echo "  all      - 编译并仿真"
	@echo "  compile  - 编译Verilog文件"
	@echo "  simulate - 运行仿真"
	@echo "  wave     - 查看波形文件"
	@echo "  syntax   - 语法检查"
	@echo "  clean    - 清理生成文件"
	@echo "  help     - 显示此帮助信息"

.PHONY: all compile simulate wave syntax clean help